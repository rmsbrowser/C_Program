#include <stdio.h>

int main( ){
  
  int linha, coluna;

  int Interacoes;
  Interacoes = 8;

  printf("Numero de Interacoes..:");
  scanf("%d", &Interacoes);
 
  printf("\n");
  linha = 1;
  while (linha < Interacoes)
  {
    printf( "\t" );
    coluna = 1;
    while (coluna < linha)
    {
      printf( "*" );
      coluna += 1;
    }
    printf( "\n" );
    linha += 1;
  }
}

