#include <stdio.h>
#include <stdlib.h>
#include "bin_file.h"
#include <time.h>

#define ASCII_LENGTH ( 257 )

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC ( 1000000 )
#endif

/* Prototypes */
Node_t* Read_Header(Bin_File_t*);
Node_t* Unpack_Tree(Bin_File_t*, int, int);
void Translate_File(Bin_File_t*, FILE*, Node_t*);
/*void Print_Tree(Node_t* Root);*/

/* Global Vars */
int G_Pseudo_EOF_Count = 0;

int main(int argc, 
  char** argv)
{
  /* Variable Declarations */
  Bin_File_t *In_FP;
  FILE *Out_FP;
  Node_t *Root = NULL;
  double Start_Time, Stop_Time, Secs;

    if ( argc != 3 )
    {
      printf("usage:  %s [inputfile] [outputfile]\n",argv[0]);
      return 1;
    }

   Start_Time = clock();

    /* Open I/O files. */
    if ( !( In_FP = Bin_Open_Read(argv[1]) ) )
    {
      printf("ERROR: Input file %s does not exist.\n", argv[1]);
      return 1;
    }

    if ( !( Out_FP = fopen(argv[2], "w") ) )
    {
      printf("ERROR: Problem opening output file %s\n", argv[2]);
      return 1;
    }

    /* Build the tree from file header */
        Root = Read_Header(In_FP);
    if ( G_Pseudo_EOF_Count < 0 )
    {
      printf("ERROR Creating Header.\n");
      return -1;
    }
        
        /* Read through file and print characters */
    Translate_File(In_FP, Out_FP, Root);

    /* Close Files */
    Bin_Close(In_FP);
    fclose(Out_FP);
    Out_FP = NULL;
    Destroy_Tree(Root);
    
    Stop_Time = clock();
    Secs = (Stop_Time - Start_Time)/CLOCKS_PER_SEC;

    printf("Decompressed file %s in %3.3e secs.\n",argv[1],Secs);

    return 0;
} /* main() */

Node_t* Read_Header(Bin_File_t* In_FP)
{
  int Bit = -1; 
  int Num_Chars = 0;
  Node_t* Root = NULL;
  
  if ( Root != NULL )
  { 
    G_Pseudo_EOF_Count = -1;
    return Root;
  }

  /* Find Pseudo EOF */
  Num_Chars = Read_Bit(In_FP, 8);

  Bit = Read_Bit(In_FP, 1);
  while (Bit == 1)
  {
    G_Pseudo_EOF_Count++;
    Bit = Read_Bit(In_FP,1);
  }

  /* Allocate space for that first internal node */
  Root = Unpack_Tree(In_FP, Num_Chars, Bit);
 
  return Root;
} /* Read_Header */

Node_t* Unpack_Tree(Bin_File_t* In_FP, 
  int Num_Chars, 
  int Bit)
{
  static int Ones_Count = 0;
  int I = 0;
  Node_t* Root;
  /* Function always receives location for next Node, 
     unallocated */
         
  Root = malloc(sizeof(Node_t));
  Root->Left_Child = NULL;
  Root->Right_Child = NULL;
  
  if (Bit == 1)
  {
    Ones_Count++;
    if (Ones_Count <= Num_Chars)
    {
      Root->Char = Read_Bit(In_FP,8);
    }
    else /* Pseudo_EOF */
    {
      Root->Char = 257;
      /*Read out Pseudo EOF*/
      while (I < G_Pseudo_EOF_Count)
      {
        Bit = Read_Bit(In_FP, 1);
        I++;
      }
    }
  }
  else
  { /*Internal node*/
    Root->Char = 258;
    /*Call the function on the left, then the right child*/
    Bit = Read_Bit(In_FP, 1);
    Root->Left_Child = Unpack_Tree(In_FP, Num_Chars, Bit);
    Bit = Read_Bit(In_FP, 1);
    Root->Right_Child = Unpack_Tree(In_FP, Num_Chars, Bit);
  }
  
  return Root;  
} /* Unpack_Tree() */

void Translate_File(Bin_File_t* In_FP, 
  FILE* Out_FP, 
  Node_t* Root)
{
  Node_t* Current;
  int Bit;

  Current = Root;

  while (! feof(In_FP->FP))
  {
    
    Bit = Read_Bit(In_FP, 1);
    if (Bit == 0)
        {
          Current = Current->Left_Child;
        }
        else if (Bit == 1)
        {
          Current = Current->Right_Child;
        }
        else
        {
          printf("This done broke.\n");
        }
        
        if ( Current->Char != 258 )
        {
          if ( Current->Char == 257 )
          {
                return;
          }
          fputc(Current->Char, Out_FP);
          Current = Root;
        }
  }
} /* Translate_File() */

