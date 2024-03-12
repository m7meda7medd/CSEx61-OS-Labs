#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
typedef struct Matrix_struct
{
  int **matrix;
  int row;
  int col;
} Matrix_t;

typedef struct Matrices
{
  Matrix_t *m1, *m2, *res;
  int id;
  int row_id;
  int col_id;
} Param_t;
void *Multiply_Two_Matrices (void *param);
void *Multiply_Row_by_Matrix (void *param);
void *Multiply_Row_by_Col (void *param);
int Resolve_File_Name (char *file_name);
int Read_Matrix (int fd, Matrix_t * matrix_1);
void Free_Matrix (Matrix_t * l_matrix);
void Print_Matrix (const Matrix_t * l_matrix);

void
main (int argc, char *argv[])
{
  int fd1, fd2, o_fd1, o_fd2, o_fd3;
  int err;
  int stdout_fd, saved_fd;
  char ch;
  struct timeval stop, start;

  Matrix_t matrix1, matrix2, result;
  result.matrix = NULL;

  if (argc == 4)		// convention naming 
    {
      fd1 = Resolve_File_Name (argv[1]);
      fd2 = Resolve_File_Name (argv[2]);
      o_fd1 = Resolve_File_Name (argv[3]);
    }
  else
    {
      perror
	("Add arguments as Mat1_filename Mat_2filename  result_filename\n");
      exit (1);

    }
  Read_Matrix (fd1, &matrix1);
  Read_Matrix (fd2, &matrix2);

  if (matrix1.col != matrix2.row)
    {
      perror ("Error: Incompatible matrices for multiplication\n");
      exit (1);
    }

  result.row = matrix1.row;
  result.col = matrix2.col;
  // Allocate memory for  result matrix
  result.matrix = (int **) malloc (result.row * sizeof (int *));
  for (int i = 0; i < result.row; i++)
    {
      result.matrix[i] = (int *) malloc (result.col * sizeof (int));
    }
  printf
    ("1 --> thread per matrix \n2 --> thread per row \n3 --> thread per element \n");


  while ((ch = getchar ()))
    {
      getchar ();
      if (ch == '1')
	{
	  gettimeofday (&start, NULL);	//start checking time
	  pthread_t MxM_thread;
	  Param_t arg = {.m1 = &matrix1,.m2 = &matrix2,.res = &result,.id = 1
	  };
	  if (pthread_create
	      (&MxM_thread, NULL, Multiply_Two_Matrices, (void *) &(arg)))
	    {
	      perror ("Error Creating Thread\n");
	    }
//Print_Matrix(&matrix1) ;
//Print_Matrix(&matrix2) ;
	  pthread_join (MxM_thread, NULL);
	  gettimeofday (&stop, NULL);	//end checking time                    
	  printf ("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	  printf ("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
	}
      else if (ch == '2')
	{
	  gettimeofday (&start, NULL);	//start checking time
	  pthread_t *threads =
	    (pthread_t *) malloc (sizeof (pthread_t) * matrix1.row);
	  Param_t* args= malloc(sizeof(Param_t)* matrix1.row) ;
	  for (int t = 0; t < matrix1.row; t++)
	    {
	   args[t].m1 = &matrix1 ;
           args[t].m2 = &matrix2,
           args[t].res =&result,
           args[t].row_id = t ;

	      err =
		pthread_create (&threads[t], NULL, Multiply_Row_by_Matrix,
				(void *) &args[t]);
	      if (err)
		{
		  perror ("ERROR :  pthread_create()\n");
		  exit (-1);
		}
	    }
//Print_Matrix(&matrix1) ;
//Print_Matrix(&matrix2) ;

	  for (int t = 0; t < matrix1.row; t++)
	    {
	      pthread_join (threads[t], NULL);
	    }
	  free(args) ;
	  gettimeofday (&stop, NULL);	//end checking time

	  printf ("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
	  printf ("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);

	}
      else if (ch == '3')
	{
	  int row_index = 0, col_index = 0;
	  gettimeofday (&start, NULL);	//start checking time
	  pthread_t *threads =
	    (pthread_t *) malloc (sizeof (pthread_t) *
				  (result.row * result.col));

	   Param_t* args= malloc(sizeof(Param_t)* matrix1.row * matrix2.col) ;

	  for (int t = 0; t < (matrix1.row * matrix2.col); t++)
	    {
	   args[t].m1 = &matrix1 ;
           args[t].m2 = &matrix2,
           args[t].res =&result,
           args[t].row_id = row_index ;
	   args[t].col_id = col_index ;
	      err =
		pthread_create (&threads[t], NULL, Multiply_Row_by_Col,
				(void *) &args[t]);
	      if (err)
		{
		  perror ("ERROR :  pthread_create()\n");
		  exit (-1);
		}

	      col_index++;
	      if (col_index == matrix2.col)
		{
		  row_index++;
		  col_index = 0;
		}
	    }
//Print_Matrix(&matrix1) ;
//Print_Matrix(&matrix2) ;
	  for (int t = 0; t < (matrix1.row * matrix2.col); t++)
	    {
	      pthread_join (threads[t], NULL);
	    }
	  gettimeofday (&stop, NULL);	//end checking time
	  free(args);
	  printf ("Seconds taken:%lu\n", stop.tv_sec - start.tv_sec);
	  printf ("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);


	}
      else if (ch == 'q')
	{
	  break;
	}
      else
	{
	  perror ("Please choose from 1 , 2 , 3\n");
	}

      if (result.matrix != NULL)
	{
	  stdout_fd = fileno (stdout);
	  saved_fd = dup (stdout_fd);

	  if ((dup2 (o_fd1, stdout_fd) != -1))
	    {
	      switch (ch)
		{
		case '1':
		  printf ("method : thread per matrix\n");
		  break;
		case '2':
		  printf ("method : thread per row\n");
		  break;
		case '3':
		  printf ("method : thread per element\n");
		  break;
		default:
		  {
		  }

		}
	      Print_Matrix (&result);
	    }
	  if (dup2 (saved_fd, stdout_fd) == -1)
	    {
	      perror ("Error with dup2 ");
	    }
	}

      printf
	("1 --> thread per matrix \n1 --> thread per row \n3 --> thread per element \n");
    }

  Free_Matrix (&result);

  close (fd1);
  close (fd2);
  close (o_fd1);

  Free_Matrix (&matrix1);
  Free_Matrix (&matrix2);

}

void *
Multiply_Two_Matrices (void *param)
{
  Param_t *Data = (Param_t *) param;
  if (Data->m1->col != Data->m2->row)
    {
      perror ("Error: Incompatible matrices for multiplication\n");
      exit (1);
    }

  // Perform matrix multiplication
  for (int i = 0; i < Data->m1->row; i++)
    {
      for (int j = 0; j < Data->m2->col; j++)
	{
	  Data->res->matrix[i][j] = 0;
	  for (int k = 0; k < Data->m1->col; k++)
	    {
	      Data->res->matrix[i][j] +=
		Data->m1->matrix[i][k] * Data->m2->matrix[k][j];
	    }
	}
    }
  return NULL;
}


void *
Multiply_Row_by_Matrix (void *param)
{
  Param_t *Data = (Param_t *) param;
  // Perform matrix multiplication
  for (int j = 0; j < Data->m2->col; j++)
    {
      Data->res->matrix[Data->row_id][j] = 0;
      for (int k = 0; k < Data->m1->col; k++)
	{
	  Data->res->matrix[Data->row_id][j] +=
	    Data->m1->matrix[Data->row_id][k] * Data->m2->matrix[k][j];
	}
    }
  return NULL;
}

void *
Multiply_Row_by_Col (void *param)
{
  Param_t *Data = (Param_t *) param;
  // Perform matrix multiplication
  Data->res->matrix[Data->row_id][Data->col_id] = 0;
  for (int k = 0; k < Data->m1->col; k++)
    {
      Data->res->matrix[Data->row_id][Data->col_id] +=
	Data->m1->matrix[Data->row_id][k] * Data->m2->matrix[k][Data->col_id];
    }
  return NULL;
}

void
Free_Matrix (Matrix_t * l_matrix)
{
  for (int i = 0; i < l_matrix->row; i++)
    {
      free (l_matrix->matrix[i]);
    }
  free (l_matrix->matrix);
  l_matrix->matrix = NULL;

}

void
Print_Matrix (const Matrix_t * l_matrix)
{
  if (l_matrix->matrix != NULL)
    {
      printf ("row = %d , col= %d\n", l_matrix->row, l_matrix->col);
      for (int i = 0; i < l_matrix->row; i++)
	{
	  for (int j = 0; j < l_matrix->col; j++)
	    {
	      printf ("%d ", l_matrix->matrix[i][j]);

	    }
	  printf ("\n");
	}
      printf ("\n");
    }
}



int
Read_Matrix (int fd, Matrix_t * matrix_1)
{
  int num_read;
  int flag = 0;
  int row, col;
  int i = 0, j = 0;
  char buf[512];
  char *temp_str, *token;
  while ((num_read = read (fd, buf, 512)))
    {
      if (num_read == -1)
	{
	  perror ("can't read\n");
	  exit (1);
	}
      temp_str = buf;
      while (token = strtok (temp_str, " \n"))
	{
	  if (strncmp (token, "row=", 4) == 0)
	    {
	      matrix_1->row = atoi (&token[4]);
	      row = matrix_1->row;
	      flag++;
	    }
	  else if (strncmp (token, "col=", 4) == 0)
	    {
	      matrix_1->col = atoi (&token[4]);
	      col = matrix_1->col;
	      if ((row != 0) && (col != 0))
		{
		  matrix_1->matrix =
		    (int **) realloc (matrix_1->matrix, sizeof (int *) * row);
		  if (matrix_1->matrix == NULL)
		    {
		      printf ("1\n");
		      while (1);
		    }
		  for (int m = 0; m < row; m++)
		    {
		      matrix_1->matrix[m] = NULL;
		    }
		  for (int m = 0; m < row; m++)
		    {
		      matrix_1->matrix[m] =
			(int *) realloc (matrix_1->matrix[m],
					 sizeof (int) * col);
		      if (matrix_1->matrix[m] == NULL)
			{
			  printf ("2\n");
			  while (1);
			}
		    }
		  flag++;
		}
	    }
	  else
	    {
	      if (flag == 2)
		{
		  matrix_1->matrix[i][j] = atoi (token);
		  j++;
		  if (j == col)
		    {
		      i++;
		      if ((i == row) && (j == col))
			{
			  break;
			}
		      j = 0;
		    }
		}
	      else
		{
		  flag = -1;
		  break;
		}
	    }
	  temp_str = NULL;
	}
      if (flag == -1)
	{
	  perror ("invalid row or col\n");
	  return -1;
	}
    }
  return 0;
}

int
Resolve_File_Name (char *file_name)
{
  int fd;
  char *l_filename = NULL;
  l_filename =
    (char *) realloc (l_filename, (strlen (file_name) + strlen (".txt") + 1));
  if ((l_filename != NULL))
    {
      strcpy (l_filename, file_name);
      strcat (l_filename, ".txt");

      fd = open (l_filename, O_RDWR | O_CREAT, 0666);
      if ((fd > 0))
	{
	  printf ("%s:open success\n", l_filename);

	  return fd;
	}
    }
  return -1;			// return -1 in case or error 
}
