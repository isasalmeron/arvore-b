/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Estruturas de Dados 2
 * Prof. Tiago A. de Almeida
 *
 * Trabalho 02 - Árvore B
 *
 * RA: 552593
 * Aluno: Isabela Salmeron Boschi

 * OBS.: Algumas idéias deste trabalho foram discutidas com o aluno
   Ricardo Mendes.
 * ========================================================================== */


/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


/* Tamanho dos campos dos registros */
#define	TAM_PRIMARY_KEY	13
#define	TAM_NOME		38
#define	TAM_TIPO		38
#define TAM_CP			8
#define	TAM_DATA		9
#define	TAM_HORA		6
#define	TAM_NIVEL		4

#define TAM_REGISTRO    192
#define MAX_REGISTROS   10000
#define TAM_ARQUIVO     (MAX_REGISTROS * TAM_REGISTRO + 1)


/* Saídas do usuário */
#define OPCAO_INVALIDA          "Opcao invalida!\n"
#define MEMORIA_INSUFICIENTE    "Memoria insuficiente!"
#define REGISTRO_N_ENCONTRADO   "Registro nao encontrado!\n\n"
#define CAMPO_INVALIDO          "Campo invalido! Informe novamente.\n"
#define ERRO_PK_REPETIDA        "ERRO: Ja existe um registro com a chave primaria: %s.\n"
#define ARQUIVO_VAZIO           "Arquivo vazio!"
#define NOS_PERCORRIDOS         "Busca por %s. Nos percorridos:\n"


/* Registro do Pokémon */
typedef struct {
	char primary_key[TAM_PRIMARY_KEY];
	char nome_pokemon[TAM_NOME];
	char tipo_pokemon[TAM_TIPO];
	char combat_points[TAM_CP];
	char data_captura[TAM_DATA]; 	// DD/MM/AA
	char hora_captura[TAM_HORA];	// HH:MM
	char nome_treinador[TAM_NOME];
	char nivel_treinador[TAM_NIVEL];
	char nome_equipe[TAM_NOME];
} Pokemon;


/* Registro da Árvore-B
 * Contém a chave primária e o RRN do respectivo registro */
typedef struct {
	char pk[TAM_PRIMARY_KEY];	// chave primária
	int rrn;					// rrn do registro
} Chave;


/* Estrutura da Árvore-B */
typedef struct node node_Btree;
struct node {
	int num_chaves;		// numero de chaves armazenadas
	Chave *chave;		// vetor das chaves e rrns [m-1]
	node_Btree **desc;	// ponteiros para os descendentes, *desc[m]
	int folha;			// flag folha da arvore
};

typedef struct {
	node_Btree *raiz;
} Iprimary;


/* Registro do índice secundário - ipokemon.idx
 * Contém o nome do Pokémon e a chave primária do registro */
typedef struct {
	char nome_pokemon[TAM_NOME];
	char primary_key[TAM_PRIMARY_KEY];
} Ipokemon;


/* Registro do índice secundário - iteam.idx
 * Contém o nome da equipe e a chave primária do registro */
typedef struct {
	char nome_equipe[TAM_NOME];
	char primary_key[TAM_PRIMARY_KEY];
} Iteam;


/* Variáveis globais */
char ARQUIVO[TAM_ARQUIVO];
int M;

/* ==========================================================================
 * ========================= PROTÓTIPOS DAS FUNÇÕES =========================
 * ========================================================================== */

/* Recebe do usuário uma string simulando o arquivo completo e retorna o número
 * de registros. */
int carregar_arquivo();

/* Exibe o Pokémon */
void exibir_registro(int rrn);

void ignore();

/* ---------- FUNÇÕES DA ÁRVORE ---------- */
int arvoreVazia(Iprimary *iprimary);
node_Btree *criaNo();
node_Btree *divideNo(node_Btree *no, Chave chave, Chave *chavePromovida, node_Btree *filhoDireito, int *flag);
node_Btree *inserirAux(node_Btree *no, Chave chave, Chave *chavePromovida, int *flag);
void preorder(node_Btree *no, int altura);
void inserir(Iprimary *iprimary, Chave chave);
node_Btree *buscaArvore(node_Btree *no, char pk[], int *posicao);
node_Btree *buscaArvore2(node_Btree *no, char pk[], int *posicao);

/* ---------- FUNÇÕES PRINCIPAIS ---------- */
void criar_iprimary(Iprimary *iprimary, int nregistros, int ordem);
void criar_ipokemon(Ipokemon *ipokemon, int nregistros);
void criar_iteam(Iteam *iteam, int nregistros);
void cadastrar(Iprimary *iprimary, Ipokemon *ipokemon, Iteam *iteam, int *nregistros);
void alterar(Iprimary iprimary);
void buscar(Iprimary iprimary, Ipokemon *ipokemon, Iteam *iteam, int nregistros);
void listar(Iprimary iprimary, Ipokemon *ipokemon, Iteam *iteam, int nregistros);
void apagar_no(node_Btree **raiz);

/* ---------- FUNÇÕES AUXILIARES ---------- */
int verificaNome(char nome[]);
void converteMaiusculo(char str[]);
int verificaTipo(char nome[]);
int verificaCp(char str[]);
int verificaData(int dia, int mes, int ano);
int verificaHora(int hora, int minuto);
int verificaNivel(char str[]);
int verificaNomeEquipe(char nome[]);
int verificaNomeTreinador(char nome[]);
void chavePrimaria(Pokemon *pokemon);
Pokemon recuperar_registro(int rrn);
void ordenaVetorPokemon(Ipokemon *ipokemon, int nregistros);
void ordenaVetorEquipe(Iteam iteam[], int nregistros);
void ordenaVetorChavePokemon(Ipokemon vet[], int nregistros);
void ordenaVetorChaveEquipe(Iteam vet[], int nregistros);

/* ==========================================================================
 * ============================ FUNÇÃO PRINCIPAL ============================
 * =============================== NÃO ALTERAR ============================== */
int main() {
	int i;
	/* Arquivo */
	int carregarArquivo = 0, nregistros = 0;
	scanf("%d\n", &carregarArquivo); // 1 (sim) | 0 (nao)
	if (carregarArquivo) {
		nregistros = carregar_arquivo();
	}

	/* Índice primário */
	int ordem;
	scanf("%d", &ordem);
	Iprimary iprimary;
	criar_iprimary(&iprimary, nregistros, ordem);

	/* Índice secundário de nomes de Pokémon */
	Ipokemon *ipokemon = (Ipokemon *) malloc (MAX_REGISTROS * sizeof(Ipokemon));
	if (!ipokemon) {
		perror(MEMORIA_INSUFICIENTE);
		exit(1);
	}
	criar_ipokemon(ipokemon, nregistros);

	/* Índice secundário de equipes */
	Iteam *iteam = (Iteam *) malloc (MAX_REGISTROS * sizeof(Iteam));
	if (!iteam) {
		perror(MEMORIA_INSUFICIENTE);
		exit(1);
	}
	criar_iteam(iteam, nregistros);

	/* Execução do programa */
	int opcao = 0;
	while(opcao != 5) {
		scanf("%d", &opcao);
		switch(opcao) {
		// Cadastrar um novo Pokémon
		case 1:
			getchar();
			cadastrar(&iprimary, ipokemon, iteam, &nregistros);
			break;
		// Alterar os combat points de um Pokémon
		case 2:
			getchar();
			alterar(iprimary);
			break;
		// Buscar um Pokémon
		case 3:
			buscar(iprimary, ipokemon, iteam, nregistros);
			break;
		// Listar todos os Pokémons
		case 4:
			listar(iprimary, ipokemon, iteam, nregistros);
			break;
		// Liberar a memória alocada
		case 5:
			apagar_no(&iprimary.raiz);
			free(ipokemon);
			free(iteam);
			break;
        // Imprimir o arquivo de dados
		case 10:
			printf("%s\n", ARQUIVO);
			break;
        // Ignora a entrada e exibe mensagem de erro
		default:
			ignore();
			printf(OPCAO_INVALIDA);
			break;
		}
	}
	return 0;
}

/* ==========================================================================
 * =============================== AUXILIARES ===============================
 * ========================================================================== */

/* Descarta o que estiver no buffer de entrada */
void ignore() {
	char c;
	while ((c = getchar()) != '\n' && c != EOF);
}

//Função que verifica se o nome do pokemon é válido, ou seja, se contém apenas letras
int verificaNome(char nome[]){
	int i = 0;

   	//Percorre a string char a char, se encontrar um caractere que não seja letra, sai da função
   	while(nome[i] != '\0'){
      	if(!((nome[i] >= 'A' && nome[i] <= 'Z') || (nome[i] >= 'a' && nome[i] <= 'z'))){
         	return 0;
      	}
      	i++;
   	}

   return 1;
}

//Função que converte string para caracteres maiusculos
void converteMaiusculo(char str[]){
   	int i = 0;

   	while(str[i] != '\0'){
      	str[i] = toupper(str[i]);
      	i++;
   	}
}

//Função que verifica se o tipo do pokemon é válido
int verificaTipo(char nome[]){
	int i = 0, flag = 0;

   	//Percorre a string char a char, se encontrar um caractere que não seja letra ou '/', sai da função
   	while(nome[i] != '\0'){
      	if((i != 0 && nome[i+1] != '\0') && nome[i] == '/'){
      		flag++;
         	i++;
      	}
      	else if(!((nome[i] >= 'A' && nome[i] <= 'Z') || (nome[i] >= 'a' && nome[i] <= 'z')) || flag == 2){
         	return 0;
      	}
      	else{
         	i++;
      	}
   	}

   	return 1;
}

//Função que verifica se o cp do pokemon é válido
int verificaCp(char str[]){
   	int i = 0, flag = 0;

   	if(strlen(str) != TAM_CP-1){
   		return 0;
   	}

   	while(str[i] != '\0'){
      	if(!((str[i] >= '0' && str[i] <= '9') || str[i] == '.')){
         	return 0;
      	}
      	if(str[i] == '.'){
         	flag = 1;
      	}
      	if(str[i] == '.' && i != 4){
         	return 0;
      	}
      	i++;
   }

   	if(flag == 0){
      	return 0;
   	}
   	return 1;
}

//Função que verifica se a data de captura é válida
int verificaData(int dia, int mes, int ano){

   if(ano < 16 || dia < 1 || mes < 1 || mes > 12){
      return 0;
   }
   if((mes == 1 || mes == 3 || mes == 5 || mes == 7 || mes == 8 || mes == 10 || mes == 12) && (dia > 31)){
      return 0;
   }
   if((mes == 4 || mes == 6 || mes == 9 || mes == 11) && (dia > 30)){
      return 0;
   }
   //Se o ano for bissexto, o mês for fevereiro e o dia for maior que 29
   if((ano % 4 == 0) && (mes == 2) && (dia > 29)){
      return 0;
   }
   //Se o ano não for bissexto, o mês for fevereiro e o dia for maior que 28
   if((ano % 4 != 0) && (mes == 2) && (dia > 28)){
      return 0;
   }

   return 1;
}

//Função que verifica se o horário de captura é válido
int verificaHora(int hora, int minuto){
   	if((hora < 0 || hora > 23) || (minuto < 0 || minuto > 59)){
      	return 0;
   	}

   	return 1;
}

//Função que verifica se o nível do treinador é válido
int verificaNivel(char str[]){
   	int num;

   	//Calcula a quantidade de bytes do numero, se não for 3 é inválido
   	if(strlen(str) != 3){
      	return 0;
   	}

   	num = atoi(str);  //Converte a string para int 
   	//Se não for um número entre 1 e 100, é inválido
   	if(num < 1 || num > 100){
      	return 0;
   	}

   	return 1;
}

//Função que verifica se o nome da equipe é válido
int verificaNomeEquipe(char nome[]){
   	converteMaiusculo(nome);   //Converte para maiusculo
   	if(strcmp(nome, "VALOR") == 0){
      	return 1;
   	}
   	if(strcmp(nome, "INSTINCT") == 0){
      	return 1;
   	}
   	if(strcmp(nome, "MYSTIC") == 0){
      	return 1;
   	}

   return 0;
}

//Função que verifica se o nome do treinador é válido
int verificaNomeTreinador(char nome[]){
	int i = 0;

   	//Percorre a string char a char, se encontrar um caractere que não seja letra, sai da função
   	while(nome[i] != '\0'){
      	if(!((nome[i] >= 'A' && nome[i] <= 'Z') || (nome[i] >= 'a' && nome[i] <= 'z') || (nome[i] >= '0' && nome[i] <= '9'))){
         	return 0;
      	}
      	i++;
   	}

   return 1;
}

//Função que cria a chave primária
void chavePrimaria(Pokemon *pokemon){

   pokemon->primary_key[0] = pokemon->nome_equipe[0]; //Primeira letra do nome da equipe
   pokemon->primary_key[1] = pokemon->nome_treinador[0]; //Primeira letra do nome do treinador
   pokemon->primary_key[2] = pokemon->nome_pokemon[0];   //Primeira letra do nome do pokemon
   pokemon->primary_key[3] = pokemon->nome_pokemon[1];   //Segunda letra do nome do pokemon
   pokemon->primary_key[4] = pokemon->data_captura[0];	//Primeiro dígito do dia
   pokemon->primary_key[5] = pokemon->data_captura[1];	//Segundo dígito do dia
   pokemon->primary_key[6] = pokemon->data_captura[3];	//Primeiro digito do mês
   pokemon->primary_key[7] = pokemon->data_captura[4];	//Segundo dígito do mês
   pokemon->primary_key[8] = pokemon->hora_captura[0];	//Primeiro dígito da hora
   pokemon->primary_key[9] = pokemon->hora_captura[1];	//Segundo dígito da hora
   pokemon->primary_key[10] = pokemon->hora_captura[3];	//Primeiro dígito do minuto
   pokemon->primary_key[11] = pokemon->hora_captura[4];	//Segundo dígito minuto
   pokemon->primary_key[12] = '\0';
}

//Função que ordena vetor por nome de pokemon
void ordenaVetorPokemon(Ipokemon *ipokemon, int nregistros){
   int i, j;
   Ipokemon aux;

   for(i = 0; i < nregistros; i++){
      for(j = i+1; j < nregistros; j++){
         if(strcmp(ipokemon[i].nome_pokemon, ipokemon[j].nome_pokemon) > 0){
            aux = ipokemon[i];
            ipokemon[i] = ipokemon[j];
            ipokemon[j] = aux;
         }
         else if(strcmp(ipokemon[i].nome_pokemon, ipokemon[j].nome_pokemon) == 0 && (strcmp(ipokemon[i].primary_key, ipokemon[j].primary_key) > 0)){
            aux = ipokemon[i];
            ipokemon[i] = ipokemon[j];
            ipokemon[j] = aux;
         }
      }
   }
}

//Função que ordena vetor por nome de equipe
void ordenaVetorEquipe(Iteam iteam[], int nregistros){
   int i, j;
   Iteam aux;

   for(i = 0; i < nregistros; i++){
      for(j = i+1; j < nregistros; j++){
         if(strcmp(iteam[i].nome_equipe, iteam[j].nome_equipe) > 0){
            aux = iteam[i];
            iteam[i] = iteam[j];
            iteam[j] = aux;
         }
         else if(strcmp(iteam[i].nome_equipe, iteam[j].nome_equipe) == 0 && (strcmp(iteam[i].primary_key, iteam[j].primary_key) > 0)){
            aux = iteam[i];
            iteam[i] = iteam[j];
            iteam[j] = aux;
         }
      }
   }
}

//Função que ordena vetor por chave primária
void ordenaVetorChavePokemon(Ipokemon vet[], int nregistros){
   int i, j;
   Ipokemon aux;

   for(i = 0; i < nregistros; i++){
      for(j = i+1; j < nregistros; j++){
         if(strcmp(vet[i].primary_key, vet[j].primary_key) > 0){
            aux = vet[i];
            vet[i] = vet[j];
            vet[j] = aux;
         }
      }
   }
}

//Função que ordena vetor por chave primária
void ordenaVetorChaveEquipe(Iteam vet[], int nregistros){
   int i, j;
   Iteam aux;

   for(i = 0; i < nregistros; i++){
      for(j = i+1; j < nregistros; j++){
         if(strcmp(vet[i].primary_key, vet[j].primary_key) > 0){
            aux = vet[i];
            vet[i] = vet[j];
            vet[j] = aux;
         }
      }
   }
}

/* ==========================================================================
 * ================================= FUNÇÕES ================================
 * ========================================================================== */

/* Recebe do usuário uma string simulando o arquivo completo e retorna o número
 * de registros. */
int carregar_arquivo() {
	scanf("%[^\n]\n", ARQUIVO);
	return strlen(ARQUIVO) / TAM_REGISTRO;
}

/* Exibe o Pokémon */
void exibir_registro(int rrn) {

	Pokemon p = recuperar_registro(rrn);

	printf("%s\n", p.primary_key);
	printf("%s\n", p.nome_pokemon);
	printf("%s\n", p.tipo_pokemon);
	printf("%s\n", p.combat_points);
	printf("%s\n", p.data_captura);
	printf("%s\n", p.hora_captura);
	printf("%s\n", p.nome_treinador);
	printf("%s\n", p.nivel_treinador);
	printf("%s\n", p.nome_equipe);
	printf("\n");
	
}

/* ---------- FUNÇÕES DA ÁRVORE ---------- */

//Função que verifica se a árvore está vazia. Se estiver retorna 1, caso contrário retorna 0
int arvoreVazia(Iprimary *iprimary){
	if(iprimary->raiz == NULL){
		return 1;
	}

	return 0;
}

//Função que cria um novo nó da árvore
node_Btree *criaNo(){
	node_Btree *novo;

	//Aloca os espaços
	novo = (node_Btree*)malloc(sizeof(node_Btree) * 1);
	novo->chave = (Chave*)malloc((M-1) * sizeof(Chave));
	novo->desc = (node_Btree**)malloc(M * sizeof(node_Btree*));

	return novo;
}

node_Btree *divideNo(node_Btree *no, Chave chave, Chave *chavePromovida, node_Btree *filhoDireito, int *flag){
	int i,j, chaveAlocada = 0;
	node_Btree *novo;

	if(no == NULL){
		*flag = 0;

		return NULL;
	}

	i = no->num_chaves - 1;
	novo = criaNo();
	novo->folha = no->folha;
	novo->num_chaves = ((M - 1) / 2);

	for(j = novo->num_chaves-1; j >= 0; j--){
		if((chaveAlocada == 0) && (strcmp(chave.pk, no->chave[i].pk) > 0)){
			novo->chave[j] = chave;
			novo->desc[j+1] = filhoDireito;
			chaveAlocada = 1;
		}
		else{
			novo->chave[j] = no->chave[i];
			novo->desc[j+1] = no->desc[i+1];
			i--;
		}
	}

	if(chaveAlocada == 0){
		while((i >= 0) && (strcmp(chave.pk, no->chave[i].pk) < 0)){
			no->chave[i+1] = no->chave[i];
			no->desc[i+2] = no->desc[i+1];
			i--;
		}
		no->chave[i+1] = chave;
		no->desc[i+2] = filhoDireito;
	}

	*chavePromovida = no->chave[M/2];
	novo->desc[0] = no->desc[(M/2)+1];
	no->num_chaves = (M/2);
	*flag = 1;

	return novo;
}

node_Btree *inserirAux(node_Btree *no, Chave chave, Chave *chavePromovida, int *flag){
	int i;
	node_Btree *filhoDireito = NULL;

	if(no == NULL){
		*flag = 0;
		return NULL;
	}

	if(no->folha == 1){
		if(no->num_chaves < (M - 1)){
			i = no->num_chaves - 1;

			while((i >= 0) && (strcmp(chave.pk, no->chave[i].pk) < 0)){
				no->chave[i+1] = no->chave[i];
				i--;
			}

			no->chave[i+1] = chave;
			no->num_chaves = no->num_chaves + 1;
			*flag = 0;

			return NULL;
		}
		else{
			return divideNo(no, chave, chavePromovida, NULL, flag);
		}
	}
	else{
		i = no->num_chaves-1;

		while((i >= 0) && (strcmp(chave.pk, no->chave[i].pk) < 0)){
			i--;
		}

		i++;
		filhoDireito = inserirAux(no->desc[i], chave, chavePromovida, flag);

		if(*flag == 1){
			chave = *chavePromovida;

			if(no->num_chaves < (M - 1)){				
				i = no->num_chaves - 1;

				while((i >= 0) && (strcmp(chave.pk, no->chave[i].pk) < 0)){
					no->chave[i+1] = no->chave[i];
					no->desc[i+2] = no->desc[i+1];
					i--;
				}

				no->chave[i+1] = chave;
				no->desc[i+2] = filhoDireito;
				no->num_chaves = no->num_chaves + 1;
				*flag = 0;

				return NULL;
			}
			else{
				return divideNo(no, chave, chavePromovida, filhoDireito, flag);
			}
		}
		else{
			*flag = 0;

			return NULL;
		}
	}
}

void inserir(Iprimary *iprimary, Chave chave){
	Chave chavePromovida;
	node_Btree *filhoDireito = NULL, *novo;
	int flag = 0;

	if(arvoreVazia(iprimary)){
		novo = criaNo();
		novo->num_chaves = 1;
		novo->folha = 1;
		novo->chave[0] = chave;
		novo->desc[0] = NULL;
		novo->desc[1] = NULL;
		iprimary->raiz = novo;
	}
	else{
		filhoDireito = inserirAux(iprimary->raiz, chave, &chavePromovida, &flag);

		if(flag == 1){
			novo = criaNo();
			novo->num_chaves = 1;
			novo->folha = 0;	
			novo->chave[0] = chavePromovida;
			novo->desc[0] = iprimary->raiz;
			novo->desc[1] = filhoDireito;
			iprimary->raiz = novo;
		}
	}
}

//Função que busca uma chave na árvore
node_Btree *buscaArvore(node_Btree *no, char pk[], int *posicao){
	int i = 0;

	if(no == NULL){
		return NULL;
	}

	while((i < no->num_chaves) && (strcmp(pk, no->chave[i].pk)  > 0)){
		i++;
	}

	if((i < no->num_chaves) && (strcmp(pk, no->chave[i].pk) == 0)){
		*posicao = i;
		return no;
	}

	if(no->folha){
		return NULL;
	}
	else{
		return buscaArvore(no->desc[i], pk, posicao);
	}
}

//Função que busca uma chave na árvore, listando os nós percorridos
node_Btree *buscaArvore2(node_Btree *no, char pk[], int *posicao){
	int i = 0;

	if(no == NULL){
		return NULL;
	}

    for(i = 0; i < no->num_chaves; i++){
        if(no->num_chaves == 1 && i == 0){
            printf("%s\n", no->chave[i].pk);
        }
        else if(i+1 == no->num_chaves){
            printf("%s\n", no->chave[i].pk);
        }
        else if(i < no->num_chaves){
            printf("%s, ", no->chave[i].pk);
        }
    }

    i = 0;
	while((i < no->num_chaves) && (strcmp(pk, no->chave[i].pk)  > 0)){
		i++;
	}

	if((i < no->num_chaves) && (strcmp(pk, no->chave[i].pk) == 0)){
		*posicao = i;
		return no;
	}

	if(no->folha){
		return NULL;
	}
	else{
		return buscaArvore2(no->desc[i], pk, posicao);
	}
}

//Realiza a busca pré ordem
void preorder(node_Btree *no, int altura){
	int i;

	if(no != NULL){
        printf("%d - ", altura);

        for(i = 0; i < M; i++){
            if(no->num_chaves == 1 && i == 0){
                printf("%s\n", no->chave[i].pk);
            }
            else if(i+1 == no->num_chaves){
                printf("%s\n", no->chave[i].pk);
            }
            else if(i < no->num_chaves){
                printf("%s, ", no->chave[i].pk);
            }
        }
        for(i = 0; i <= no->num_chaves; i++){
            preorder(no->desc[i], altura+1);
        } 
    }
}

/* ---------- FUNÇÕES PRINCIPAIS ---------- */

//Função que cria a árvore de índices primários
void criar_iprimary(Iprimary *iprimary, int nregistros, int ordem){
	Chave chave;
	int i;
	char *p = ARQUIVO, primary_key[TAM_PRIMARY_KEY];

	iprimary->raiz = NULL;
	M = ordem;

	//Percorre o arquivo de dados
	for(i = 0; i < nregistros; i++){
		p = ARQUIVO + (i * 192);
		sscanf(p, "%[^@]@", primary_key);
		strcpy(chave.pk, primary_key);	//Salva a chave primária
		chave.rrn = i;	//Sala o rrn
		inserir(iprimary, chave);	//Insere na árvore
	}
}

//Função que cria o índice secundário por nome de pokemon
void criar_ipokemon(Ipokemon *ipokemon, int nregistros){
	int i;
	Pokemon pokemon;
	char *p = ARQUIVO;

	//Percorre o arquivo de dados
	for(i = 0; i < nregistros; i++){
		p = ARQUIVO + (i * 192);
		sscanf(p,"%[^@]@%[^@]@", pokemon.primary_key, pokemon.nome_pokemon);
		strcpy(ipokemon[i].nome_pokemon, pokemon.nome_pokemon);
		strcpy(ipokemon[i].primary_key, pokemon.primary_key);
	}
}

//Função que cria o índice secundário por nome de time
void criar_iteam(Iteam *iteam, int nregistros){
	int i, cont;
	Pokemon pokemon;
	char *p = ARQUIVO, c;

	//Percorre o arquivo de dados
	for(i = 0; i < nregistros; i++){
		p = ARQUIVO + (i * 192);
		sscanf(p,"%[^@]", pokemon.primary_key);	//Lê a chave primária
		
		//Chega até o campo do time
      	cont = 0;
      	while(cont < 8){
        	sscanf(p,"%c", &c);

        	if(c == '@'){
            	cont++;
         	}
         	p++;
      	}
      	sscanf(p,"%[^@]", pokemon.nome_equipe);	//Lê o time

      	//Salva no vetor de time
		strcpy(iteam[i].nome_equipe, pokemon.nome_equipe);
		strcpy(iteam[i].primary_key, pokemon.primary_key);
	}
}

//Função que recebe os dados e faz as verificações necessárias, se estiver tudo corretor, insere.
void cadastrar(Iprimary *iprimary, Ipokemon *ipokemon, Iteam *iteam, int *nregistros){
	char aux[100], sdia[5], smes[5], sano[5], shora[3], sminuto[3], registro[193], complemento[192];
	Pokemon pokemon;
	int dia, mes, ano, hora, minuto, tam, i;
	Chave novaChave;

	//Recebe o nome do pokemon
	scanf("%s", aux);
	ignore();
	//Enquanto não for um nome válido, pede um novo nome
	while(!verificaNome(aux)){
		printf(CAMPO_INVALIDO);
		scanf("%s", aux);	
		ignore();
	}
	converteMaiusculo(aux);	//Converte os caracteres em letras maiúsculas
	strcpy(pokemon.nome_pokemon, aux); //Salva o nome do pokemon

	//Recebe o tipo do pokemon
	scanf("%s", aux);	
	ignore();
	//Enquanto não for um tipo válido, pede um novo tipo
	while(!verificaTipo(aux)){
		printf(CAMPO_INVALIDO);
		scanf("%s", aux);	
		ignore();
	}
	converteMaiusculo(aux);	//Converte os caracteres em letras maiúsculas
	strcpy(pokemon.tipo_pokemon, aux); //Salva o tipo do pokemon

	//Recebe o cp do pokemon
	scanf("%s", aux);
	ignore();	
	//Enquanto não for um cp válido, pede um novo cp
	while(!verificaCp(aux)){
		printf(CAMPO_INVALIDO);
		scanf("%s", aux);	
		ignore();
	}
	strcpy(pokemon.combat_points, aux); //Salva o cp do pokemon

	//Recebe a data de captura do pokemon
	scanf("%d/%d/%d", &dia, &mes, &ano);
	ignore();
	//Enquanto não for uma data válida, pede uma nova data
	while(!verificaData(dia, mes, ano)){
		printf(CAMPO_INVALIDO);
		scanf("%d/%d/%d", &dia, &mes, &ano);
		ignore();
	}
	//Converte os inteiros em string
	sprintf(sdia, "%.2d", dia);
	sprintf(smes, "%.2d", mes);
	sprintf(sano, "%.2d", ano);

	//Salva no registro pokemon
	strcpy(pokemon.data_captura, sdia);
	strcat(pokemon.data_captura, "/");
	strcat(pokemon.data_captura, smes);
	strcat(pokemon.data_captura, "/");
	strcat(pokemon.data_captura, sano);
   pokemon.data_captura[TAM_DATA-1] = '\0';

	//Recebe a hora de captura do pokemon
	scanf("%d:%d", &hora, &minuto);
	ignore();
	//Enquanto não for um horário válid0, pede um novo horário
	while(!verificaHora(hora, minuto)){
		printf(CAMPO_INVALIDO);
		scanf("%d:%d", &hora, &minuto);
		ignore();
	}
	//Converte os inteiros em string
	sprintf(shora, "%.2d", hora);
	sprintf(sminuto, "%.2d", minuto);

	//Salva no registro pokemon
	strcpy(pokemon.hora_captura, shora);
	strcat(pokemon.hora_captura, ":");
	strcat(pokemon.hora_captura, sminuto);
   pokemon.hora_captura[TAM_HORA-1] = '\0';

	//Recebe o nome do treinador
	scanf("%s", aux);
	ignore();
	//Enquanto não for um nome válido, pede um novo nome
	while(!verificaNomeTreinador(aux)){
		printf(CAMPO_INVALIDO);
		scanf("%s", aux);	
		ignore();
	}
	converteMaiusculo(aux);	//Converte os caracteres em letras maiúsculas
	strcpy(pokemon.nome_treinador, aux); //Salva o nome do treinador

	//Recebe o nível do treinador
	scanf("%s", aux);
	ignore();
	//Enquanto não for um nível válido, pede um novo nível
	while(verificaNivel(aux) == 0){
		printf(CAMPO_INVALIDO);
		scanf("%s", aux);	
		ignore();
	}
	strcpy(pokemon.nivel_treinador, aux);	//Salva o nível do treinador

	//Recebe o nome da equipe
	scanf("%s", aux);
	ignore();
	//Enquanto não for um nome válido, pede um novo nome
	while(!verificaNomeEquipe(aux)){
		printf(CAMPO_INVALIDO);
		scanf("%s", aux);
		ignore();
	}
	strcpy(pokemon.nome_equipe, aux); //Salva o nome da equipe

	//Cria a chave primária
   chavePrimaria(&pokemon);

   //Se a chave primária não existe
   if(buscaArvore(iprimary->raiz, pokemon.primary_key, &i) == NULL){
   	//Atualiza o indice secundario de pokemons
		strcpy(ipokemon[*nregistros].nome_pokemon, pokemon.nome_pokemon);
		strcpy(ipokemon[*nregistros].primary_key, pokemon.primary_key);

		//Atualiza o indice secundario de equipes
		strcpy(iteam[*nregistros].nome_equipe, pokemon.nome_equipe);
		strcpy(iteam[*nregistros].primary_key, pokemon.primary_key);

		//Insere o novo pokemon no índice primário
		strcpy(novaChave.pk, pokemon.primary_key);
		novaChave.rrn = *nregistros;
		inserir(iprimary, novaChave);

		//Calcula o tamanho do registro
   	tam = 44 + strlen(pokemon.nome_pokemon) + strlen(pokemon.tipo_pokemon) + strlen(pokemon.nome_treinador) + strlen(pokemon.nome_equipe);
   	tam = 192 - tam; //Calcula a quantidade de # que será necessários para preencher o registro no arquivo de dados
   	for(i = 0; i < tam; i++){
   		complemento[i] = '#';
   	}
   	complemento[i] = '\0';

		//Salva os dados no arquivo
		sprintf(registro, "%s@%s@%s@%s@%s@%s@%s@%s@%s@%s", pokemon.primary_key, pokemon.nome_pokemon, pokemon.tipo_pokemon, pokemon.combat_points, pokemon.data_captura, pokemon.hora_captura, pokemon.nome_treinador, pokemon.nivel_treinador,  pokemon.nome_equipe, complemento);
		strcat(ARQUIVO, registro);

		(*nregistros)++;
   }
   else{
    	printf(ERRO_PK_REPETIDA, pokemon.primary_key);
   }
}

void alterar(Iprimary iprimary){
	char chave[TAM_PRIMARY_KEY], nome[TAM_NOME], tipo[TAM_TIPO], cp[TAM_CP], *p;
	node_Btree *no;
	int tam, posicao;

	scanf("%[^\n]s", chave);	//Recebe a chave primária
	ignore();
	converteMaiusculo(chave);	//Converte os caracteres em letras maiúsculas

	//Se a chave primária não existir
	no = buscaArvore(iprimary.raiz, chave, &posicao);
	if(no == NULL){
    	printf(REGISTRO_N_ENCONTRADO);
	}
	//Se existir
	else{
		scanf("%[^\n]s", cp);	//Recebe o novo cp
		ignore();

		//Enquanto não for um cp válido, pede um novo cp
		while(!verificaCp(cp)){
			printf(CAMPO_INVALIDO);
			scanf("\n%[^\n]s", cp);
			cp[TAM_CP-1] = '\0';
		}

		p = ARQUIVO + (no->chave[posicao].rrn * 192) + 13;	//Vai até o registro já no campo tipo do pokemon
		sscanf(p, "%[^@]@%[^@]", nome, tipo);	//Lê o nome e o tipo do pokemon
		tam = strlen(nome) + strlen(tipo) + 2;	
		p = ARQUIVO + (no->chave[posicao].rrn * 192) + 13 + tam;	//Vai até o campo cp do registro
		
		//Altera o arquivo
		sprintf(p, "%s", cp);
		p = p + TAM_CP - 1;
		*p = '@';
	}
}

//Função que dada uma opção, realiza a busca do registro do pokemon
void buscar(Iprimary iprimary, Ipokemon *ipokemon, Iteam *iteam, int nregistros){
	int opcao, i, j = 0, posicao;
	char entrada[100];
	Ipokemon *result = (Ipokemon *) malloc (nregistros * sizeof(Ipokemon));
	Iteam *resultado = (Iteam *) malloc(nregistros * sizeof(Iteam));
	node_Btree *pokemon;

	scanf("%d", &opcao);

	switch(opcao){
		//Por código
		case 1:
			scanf("\n%[^\n]s", entrada);
			ignore();
         entrada[12] = '\0';
			converteMaiusculo(entrada);

			printf(NOS_PERCORRIDOS, entrada);
			//Busca o pokemom na árvore, listando os nós percorridos
			pokemon = buscaArvore2(iprimary.raiz, entrada, &posicao);
			printf("\n");

			//Se encontrou, exibe o registro
			if(pokemon != NULL){
				exibir_registro(pokemon->chave[posicao].rrn);
			}
			else{
				printf(REGISTRO_N_ENCONTRADO);
			}
		break;

		//Por nome do pokemon
		case 2:
			scanf("\n%[^\n]s", entrada);	//Recebe o nome do pokemon
			ignore();
			converteMaiusculo(entrada);

			//Percorre o vetor de pokemon
			for(i = 0; i < nregistros; i++){
				//Se encontrou, salva em um novo vetor
				if(strcmp(ipokemon[i].nome_pokemon, entrada) == 0){
					result[j] = ipokemon[i];
					j++;
				}
			}
			//Se encontrou algum resultado
			if(j != 0){
				//Ordena vetor por chave
				ordenaVetorChavePokemon(result, j);

				//Percorre o vetor resultado imprimindo o registro
				for(i = 0; i < j; i++){
					pokemon = buscaArvore(iprimary.raiz, result[i].primary_key, &posicao);	//Procura o pokemon na árvore
					exibir_registro(pokemon->chave[posicao].rrn);	//Exibe o registro
				}
			}
			else{
				printf(REGISTRO_N_ENCONTRADO);
			}
		break;

		//Por nome da equipe
		case 3:
			scanf("\n%[^\n]s", entrada);	//Recebe o nome do time
			ignore();
			converteMaiusculo(entrada);

			//Percorre o vetor de time
			for(i = 0; i < nregistros; i++){
				//Se encontrou, salva em um novo vetor
				if(strcmp(iteam[i].nome_equipe, entrada) == 0){
					resultado[j] = iteam[i];
					j++;
				}
			}
			//Se encontrou algum resultado
			if(j != 0){
				//Ordena vetor por chave
				ordenaVetorChaveEquipe(resultado, j);

				//Percorre o vetor resultado imprimindo o registro
				for(i = 0; i < j; i++){
					pokemon = buscaArvore(iprimary.raiz, resultado[i].primary_key, &posicao);	//Procura o pokemon na árvore
					exibir_registro(pokemon->chave[posicao].rrn);	//Exibe o registro
				}
			}
			else{
				printf(REGISTRO_N_ENCONTRADO);
			}
		break;
	}

   free(resultado);
   free(result);
}

//Função que dada uma opção exibe a árvore ou os nomes dos pokémons cadastrados
void listar(Iprimary iprimary, Ipokemon *ipokemon, Iteam *iteam, int nregistros){
	int opcao, i, posicao;
	node_Btree *pokemon;

   if(nregistros == 0){
      printf(ARQUIVO_VAZIO);
   }

	scanf("%d", &opcao);	//Recebe a opção
	switch(opcao){
		//Imprime as chaves primárias da árvore
		case 1:
			preorder(iprimary.raiz, 1);
			printf("\n");
		break;

		//Imprime os nomes dos pokemons ordenados pelo seu nome
		case 2:
			ordenaVetorPokemon(ipokemon, nregistros);	//Ordena o vetor de índice de pokemon
			//Percorre o vetor de pokemons
			for(i = 0; i < nregistros; i++){
				pokemon = buscaArvore(iprimary.raiz, ipokemon[i].primary_key, &posicao);	//Procura o pokemons na árvore
				exibir_registro(pokemon->chave[posicao].rrn);	//Exibe o registro
			}
		break;

		//Imprime os nomes dos pokemons ordenados pelo nome do time
		case 3:
			ordenaVetorEquipe(iteam, nregistros);	//Ordena o vetor de índice de time
			//Percorre o vetor de pokemons
			for(i = 0; i < nregistros; i++){
				pokemon = buscaArvore(iprimary.raiz, iteam[i].primary_key, &posicao);	//Procura o pokemons na árvore
				exibir_registro(pokemon->chave[posicao].rrn);	//Exibe o registro
			}
		break;
	}	
}

//Função que encontra um determinado registro no arquivo
Pokemon recuperar_registro(int rrn){
	Pokemon pokemon;
	char *p = ARQUIVO + (rrn * 192);

	sscanf(p,"%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@%[^@]@", pokemon.primary_key, pokemon.nome_pokemon, pokemon.tipo_pokemon, pokemon.combat_points, pokemon.data_captura, pokemon.hora_captura, pokemon.nome_treinador, pokemon.nivel_treinador, pokemon.nome_equipe);

	return pokemon;
}

void apagar_no(node_Btree **raiz){
	free(*raiz);
}