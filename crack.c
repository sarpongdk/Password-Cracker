/*
 * David Sarpong
 * Lab 3
 * CSCI 3500-01
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <crypt.h>

int time_to_quit = 0;
void* thread_entry(void* args);
void substring(char* buffer, int left_index, int right_index);
void substring_l(char* buffer, int left_index);
int work_divider(int num_threads);
void display_error(char* msg);
int crack_password_helper(char* target, char* pwd_pred, int pwd_len, int keysize, char* alphabet, struct crypt_data* data);
void crack_password(char* target, char pwd[], int keysize, struct crypt_data* data);

struct thread_args
{
   char prefix[26];
   char pwd[8];
   char* target;
   int keysize;
};

void* thread_entry(void* args)
{
   struct crypt_data data; data.initialized = 0;
   struct thread_args* input = (struct thread_args*) args;
   // printf("%s\n", input->prefix);

   for (int i = 0; i < strlen(input->prefix); i++)
   {
      if (time_to_quit == 1)
      {
         break;
      }

      input->pwd[0] = input->prefix[i];
      
      for (int j = 0; j < input->keysize; j++)
      {
         crack_password(input->target, input->pwd, j, &data);
         
         if (time_to_quit == 1)
         {
            break;
         }
      }
   }

   return NULL;
}

int main(int argc, char* argv[])
{
   // error checking number of command line args
   if (argc != 4)
   {
      display_error("Error: correct usage ./crack <num of threads> <keysize> <target password>");
   }

   // important variables
   int num_threads = atoi(argv[1]);
   int keysize = atoi(argv[2]);

   // error checking keysize
   if (keysize > 8 || keysize < 1) 
   {
      display_error("Invalid keysize");
   }
	
   struct thread_args args_thread[num_threads]; 
   pthread_t tids[num_threads];
   char* target =  argv[3];
   target[strlen(target)] = '\0';
   char pwd[keysize];
   memset(pwd, 0, sizeof(pwd));
   int work_size = work_divider(num_threads);

   if (work_size == 0)
   {
      work_size = 1;
   }

   for (int i = 0; i < num_threads; i++)
   {
      args_thread[i].keysize = keysize;
      args_thread[i].target = target;

      if (i < num_threads - 1)
      {
         substring(args_thread[i].prefix, i * work_size, (i+1) * work_size - 1);
      }
      else 
      {
         substring_l(args_thread[i].prefix, i * work_size);
      } 
     
      pthread_create(&tids[i], NULL, thread_entry, &args_thread[i]);

      if (time_to_quit == 1)
      {
         break;
      }
   }

   for (int j = 0; j < num_threads; j++)
   {
      pthread_join(tids[j], NULL);
   }

   return EXIT_SUCCESS;
}

/*
 * A method for displaying error messages to the stdout
 * @return void
 */
void display_error(char* msg)
{
   perror(msg);
   exit(EXIT_FAILURE);
}

/*
 * A method that cracks the password given the target
 * @return 1 if passwrod was found, -1 otherwise
 */
int crack_password_helper(char* target, char* pwd_pred, int pwd_len, int keysize, char* alphabet, struct crypt_data* data)
{	
   if (pwd_len >= keysize)
   {
      char salt[3] = {target[0], target[1], '\0'};	
      char* crypted_pwd_pred = crypt_r(pwd_pred, salt, data);
      
      if (strcmp(crypted_pwd_pred + 2, target + 2) == 0)
      {
         time_to_quit = 1;
         printf("Password: %s\n", pwd_pred);
         return 1;
      }
      else
      {
         return -1;
      }
   }
   else if (time_to_quit == 1)
   {
      return 1;
   }

   for (int i = 0; i < 26; i++) 
   {
      char cat[2] = {alphabet[i], '\0'};
      strcat(pwd_pred, cat);// choose
      int ret = crack_password_helper(target, pwd_pred, pwd_len + 1, keysize, alphabet, data); // explore 
			
      if (ret == 1)
      {
         return ret;
      }
			
      pwd_pred[strlen(pwd_pred) - 1] = '\0'; // unchoose
   }

   return -1;	 
}

/*
 * A method that cracks the password given the target
 * @return 1 if passwrod was found, -1 otherwise
 */
void crack_password(char* target, char pwd[], int keysize, struct crypt_data* data) 
{
   char alphabet[26] = "abcdefghijklmnopqrstuvwxyz";
   int ret = crack_password_helper(target, pwd, 0, keysize, alphabet, data);
}

int work_divider(int num_threads)
{
   return 26/num_threads;
}


void substring(char* buffer, int left_index, int right_index)
{
   char alphabet[26] = "abcdefghijklmnopqrstuvwxyz";
   char substr[26];
   int index = 0;
   
   for (int i = left_index; i <= right_index; i++)
   {
      substr[index] = alphabet[i];
      index++;
   }

   substr[index] = '\0';
   strcpy(buffer, substr);
}

void substring_l(char* buffer, int left_index)
{
   char alphabet[26] = "abcdefghijklmnopqrstuvwxyz";
   char substr[26];
   int index = 0;

   for (int i = left_index; i < strlen(alphabet); i++)
   {
      substr[index] = alphabet[i];
      index++;
   }

   substr[index] = '\0';
   strcpy(buffer, substr);
}
