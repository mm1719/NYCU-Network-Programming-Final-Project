//libraries
#include "unp.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

//structures
typedef struct {
    int connfd;
    char name[100];
    //char ip[100];
    int character;
    int points;
    int isAchieve;
} Player;

typedef struct{
    char news_content[300];
    float fluctuations[8]; // example: {1.0, 1.0, 1.3, 1.0, 0.8, 1.0, 1.0, 1.0}
    int isFake;
} News;

//constant global variables
const int MAX_PLAYERS = 2;
const char WELCOME_AND_CHARACTERS[300] = 
"Welcome! Choose your character:\n\n"
"no.0: 老百姓\n"
"\t能力: 無\n"
"\t挑戰: 無\n"
"no.1: 交大富二代\n"
"\t能力: 無法向銀行借錢, 初始積分可多獲得1百萬\n" 
"\t挑戰: 若全程只購買股票類商品, 獲得1百萬積分\n"
"no.2: 空軍上將\n" 
"\t能力: 做空可賺2倍獲利, 做多僅賺0.5倍獲利\n"
"\t挑戰: 若全程做空, 獲得1百萬積分\n"
"no.3: 虧損鬼才\n"
"\t能力: 最終結算時, 將2倍的虧損金額加至對手積分\n"
"\t挑戰: 若全程虧損且每次虧損金額不小於於現有積分之10%%, 獲得3百萬積分\n\n"
"請輸入整數(0 ~ 3)選擇角色: ";
const int TOTAL_NEWS = 30;
const char NEWS_CONTENTS[][200] = { //TBD
    "AAA",
    "BBB",
    "CCC"
};
const int NEWS_FLUCTUATIONS[][200] = { //TBD
    {1.0, 1.0, 1.3, 1.0, 0.8, 1.0, 1.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.2, 1.0},
    {1.4, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}
};
const char ITEM_NAMES[8] = {"麥當勞股票", "鑽石"}; //TBD

//global variables
int listenfd, maxfd;
fd_set allset;
Player players[MAX_PLAYERS]; //for saving active players' info
int active_players;
int current_round = 1;
int flag1, flag2, flag3, flag4, flag5; //for leaving infinite loop
int item_prices[8] = {1000, 100000}; //TBD

//functions
void handle_new();
void handle_in_round_msg(int);

int main(){
    //Socket settings
	int listenfd;
    listenfd = Socket(AF_INET, SOCK_STREAM, 0); //ask for socket
    
	struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr)); //clear socket
    servaddr.sin_family = AF_INET; //socket settings - IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //socket settings - address
    servaddr.sin_port = htons(SERV_PORT + 5); //socket settings - port
    
    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr)); //bind
    Listen(listenfd, LISTENQ); //listen
    
    for (int i = 0; i < MAX_PLAYERS; i++) 
        players[i].connfd = -1; //no active player in the beginning

    //Before the game
    for(int i = 0; i < MAX_PLAYERS; i++)
    	handle_new(); //wait for players
    
    srand(time(0));

    //Round 1
    int rand_i[3];
    while(rand_i[0] == rand_i[1] || rand_i[1] == rand_i[2] || rand_i[0] == rand_i[2])
        for(int i = 0; i < 3; i++)
            rand_i[i] = rand() % TOTAL_NEWS;

    News news_round1[3];
    for(int i = 0; i < 3; i++){
        snprintf(news_round1[i].news_content, sizeof(news_round1[i].news_content), "%s", NEWS_CONTENTS[rand_i[i]]);
        for (int j = 0; j < 8; j++) {
            news_round1[i].fluctuations[j] = NEWS_FLUCTUATIONS[rand_i[i]][j];
        }
    }

    for(int i = 0; i < MAX_PLAYERS; i++){
        if(players[i].connfd != -1){
            if(players[i].character == 1){
                players[i].points = 2000000; //init points - 交大富二代
                Writen(players[i].connfd, "Round 1\nYou have 2,000,000 points!\n", 35);
            }else{
                players[i].points = 1000000; //init points - others
                Writen(players[i].connfd, "Round 1\nYou have 1,000,000 points!\n", 35);
            }
        }
    }

    while (1) {
        if(flag1)
            break;

        fd_set rset = allset;
        Select(maxfd + 1, &rset, NULL, NULL, NULL);
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (players[i].connfd != -1 && FD_ISSET(players[i].connfd, &rset))
                handle_in_round_msg(i); //select and handle readable connfd
    }

    //Round 2

    //Round 3

    //Round 4

    //Round 5

    //After the game

	return 0;
}

void handle_new(){
	if(active_players>=MAX_PLAYERS)
		return;
	
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
	connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen); //accept
    active_players++; //add new active player
    
    char buffer1[MAXLINE];
    int tmp = Read(connfd, buffer1, MAXLINE); //read name
    buffer1[tmp] = 0; //string ending

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].connfd == -1) { //find an empty players slot
            players[i].connfd = connfd; //save connfd
            strncpy(players[i].name, buffer1, strlen(buffer1)); //save name
            //strncpy(players[i].ip, inet_ntoa(cliaddr.sin_addr), strlen(inet_ntoa(cliaddr.sin_addr))); //save ip

            Writen(players[i].connfd, WELCOME_AND_CHARACTERS, strlen(WELCOME_AND_CHARACTERS)); //send character list

            int buffer2;
            int tmp2 = Read(connfd, &buffer2, sizeof(buffer2)); //read character
            players[i].character = buffer2[0] - '0'; //save character

            memset(buffer1, 0, sizeof(buffer1)); //clear buffer1
            memset(buffer2, 0, sizeof(buffer2)); //clear buffer2

            FD_SET(connfd, &allset); //add this connfd to allset
            maxfd = max(maxfd, connfd); //update maxfd
        }
    }
}

void handle_in_round_msg(int player_i){

}
