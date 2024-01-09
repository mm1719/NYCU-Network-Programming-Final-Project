//Libraries
#include "unp.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

//Structures
typedef struct {
    int connfd;
    char name[100];
    //char ip[100];
    int character;
    int points;
    int bought_amounts[8]; // +,- : buy,sell
    int loan_dollars;
    int isFin; //bool
    int isSetFake; //bool
    int isAchieved; //bool
} Player;

typedef struct{
    char news_content[300];
    char details_content[300];
    float fluctuations[8]; //example: {0.0, 0.0, 0.3, 0.0, -0.2, 0.0, 0.0, 0.0}
} News;

//Constant Global Variables
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
const int TOTAL_NEWS = 30; //TBD
const char NEWS_CONTENTS[][200] = { //TBD
    "AAA",
    "BBB"
};
const float NEWS_FLUCTUATIONS[][200] = { //TBD
    {0.0, 0.0, 0.3, 0.0, -0.2, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.2, 0.0}
};
const char ITEM_NAMES[8] = {"麥當勞股票", "鑽石"}; //TBD
const int ITEM_INIT_PRICE[8] = {1000, 10000}; //TBD
const int OP_INFO_PRICE = 200000; //TBD
const int OP_FAKE_PRICE = 500000; //TBD

//Global Variables
int listenfd, maxfd;
fd_set allset;
Player players[MAX_PLAYERS]; //for saving active players' info
int active_players;
int current_round = 1;
int flag1, flag2, flag3, flag4, flag5; //for leaving infinite loop
int item_prices[8]; 

//Functions
void handle_new();
void handle_in_round_msg(int);
int isAllFin();
News* get_3_random_news();
int extract_instr(char*);
int isContainDollarSign(char*);

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

    //Before game
    for(int i = 0; i < MAX_PLAYERS; i++)
    	handle_new(); //wait for players

    //Round 1 - prepare phase
    News *news_round1 = get_3_random_news();

    for(int i = 0; i < MAX_PLAYERS; i++){
        if(players[i].connfd != -1){
            if(players[i].character == 1){
                players[i].points += 1000000; //init points - 交大富二代
                Writen(players[i].connfd, "Round 1\nYou have 2,000,000 points!\n", 35); //send round and points info
            }
            else
                Writen(players[i].connfd, "Round 1\nYou have 1,000,000 points!\n", 35); //send round and points info

            char buffer4[MAXLINE];
            sprintf(buffer4, "MARKET NEWS:\nNEWS 1: %s\nNEWS 2: %s\nNEWS 3: %s\n\n",
                    news_round1[0].news_content, news_round1[1].news_content, news_round1[2].news_content);
            Writen(players[i].connfd, buffer4, strlen(buffer4)); //send market news
            memset(buffer4, 0, sizeof(buffer4)); //clear buffer4
        }
    }

    //Round 1 - player phase
    while (!isAllFin()) {
        fd_set rset = allset;
        Select(maxfd + 1, &rset, NULL, NULL, NULL);
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (players[i].connfd != -1 && FD_ISSET(players[i].connfd, &rset))
                handle_in_round_msg(i); //select and handle readable connfd
    }

    //Round 1 - closing phase

    //Round 2
    News *news_round2 = get_3_random_news();

    //Round 3
    News *news_round3 = get_3_random_news();

    //Round 4
    News *news_round4 = get_3_random_news();

    //Round 5
    News *news_round5 = get_3_random_news();

    //After game

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
            //init player info
            players[i].connfd = connfd;
            strncpy(players[i].name, buffer1, strlen(buffer1));
            //strncpy(players[i].ip, inet_ntoa(cliaddr.sin_addr), strlen(inet_ntoa(cliaddr.sin_addr)));
            players[i].points = 1000000; 
            players[i].loan_dollars = 0;
            players[i].isAllFin = 0;
            players[i].isSetFake = 0;
            players[i].isAchieved = 0;

            Writen(players[i].connfd, WELCOME_AND_CHARACTERS, strlen(WELCOME_AND_CHARACTERS)); //send character list

            char buffer2[MAXLINE];
            int tmp2 = Read(connfd, buffer2, sizeof(buffer2)); //read character
            players[i].character = buffer2[0] - '0'; //save character

            memset(buffer1, 0, sizeof(buffer1)); //clear buffer1
            memset(buffer2, 0, sizeof(buffer2)); //clear buffer2

            FD_SET(connfd, &allset); //add this connfd to allset
            maxfd = max(maxfd, connfd); //update maxfd
        }
    }
}

void handle_in_round_msg(int player_i){
    char buffer3[MAXLINE];
	int nbytes = Read(players[player_i].connfd, buffer3, MAXLINE);
	buffer3[nbytes] = 0;
    
    //player leaving
    if (nbytes <= 0) {
        active_players--;
        
        if(nbytes == 0)
        	Writen(players[player_i].connfd, "Bye!\n", 5); //leave by Ctrl+D
        
        Close(players[player_i].connfd);
        FD_CLR(players[player_i].connfd, &allset);
        players[player_i].connfd = -1;
        memset(players[player_i].name, 0, sizeof(players[player_i].name)); //clear player name
		//memset(players[player_i].ip, 0, sizeof(players[player_i].ip)); //clear player ip
        
        char leaving[MAXLINE];
        if(active_players == 1){
	    	sprintf(leaving, "(%s left. You win!)\n", players[player_i].name); //prepare leaving msg
	    	for (int i = 0; i < MAX_PLAYERS; i++)
	            if (players[i].connfd != -1)
	                Writen(players[i].connfd, leaving, strlen(leaving)); //send leaving msg to last player
			memset(leaving, 0, sizeof(leaving)); //clear leaving
		}
		else{
	    	sprintf(leaving, "(%s left, %d players left)\n", players[player_i].name, active_players); //prepare leaving msg
	    	for (int i = 0; i < MAX_PLAYERS; i++)
	            if (players[i].connfd != -1)
	                Writen(players[i].connfd, leaving, strlen(leaving));  //send leaving msg to active players
			memset(leaving, 0, sizeof(leaving)); //clear leaving
		}
    }

    //player instructions
	else {
		int instr = extract_instr(buffer3);

        char not_enough[MAXLINE];
        switch (instr){
            case 1: //long with $dollar
                break;
            
            case 2: //long with amount
                break;
            
            case 3: //short with $dollar
                break;
            
            case 4: //short with amount
                break;
            
            case 5: //info
                if(players[player_i].points < OP_INFO_PRICE){
                    sprintf(not_enough, "(info operation requires %d points, you only have %d points.)\n", OP_INFO_PRICE, players[player_i].points);
                    Writen(players[player_i].connfd, not_enough, strlen(not_enough));
                }
                else{
                    players[player_i].points -= OP_INFO_PRICE;
                    for (int i = 0; i < MAX_USERS; i++)
                        if (i != player_i && players[i].connfd != -1)
                            players[i].isSetFake = 1;
                }
                break;
            
            case 6: //fake
                if(players[player_i].points < OP_FAKE_PRICE){
                    sprintf(not_enough, "(fake operation requires %d points, you only have %d points.)\n", OP_FAKE_PRICE, players[player_i].points);
                    Writen(players[player_i].connfd, not_enough, strlen(not_enough));
                }
                else{
                    players[player_i].points -= OP_FAKE_PRICE;
                    for (int i = 0; i < MAX_USERS; i++)
                        if (i != player_i && players[i].connfd != -1)
                            players[i].isSetFake = 1;
                }
                break;
            
            case 7: //loan
                break;
            
            case 8: //finish
                players[player_i].isFin = 1;
                break;
            
            // case -1: //invalid
            //     break;
        }
    }

    memset(buffer3, 0, sizeof(buffer3));
}

News* get_3_random_news(){
    srand(time(0));

    int rand_i[3]; // for randomly picking news
    while(rand_i[0] == rand_i[1] || rand_i[1] == rand_i[2] || rand_i[0] == rand_i[2])
        for(int i = 0; i < 3; i++)
            rand_i[i] = rand() % TOTAL_NEWS;

    News n[3]; //market news to send
    for(int i = 0; i < 3; i++){
        strncpy(n[i].news_content, NEWS_CONTENTS[rand_i[i]], strlen(NEWS_CONTENTS[rand_i[i]])); //save news content
        for (int j = 0; j < 8; j++) {
            n[i].fluctuations[j] = NEWS_FLUCTUATIONS[rand_i[i]][j]; //save news fluctuations
        }
    }

    return n;
}

int extract_instr(char *str) {
    char firstWord[10];

    int i = 0;
    while (str[i] != ' ' && str[i] != '\n') {
        firstWord[i] = str[i];
        i++;
    }
    firstWord[i] = 0;

    if(strcmp(firstWord, "long") == 0){
        if(isContainDollarSign(str))
            return 1;
        else
            return 2;
    }
    else if(strcmp(firstWord, "short") == 0){
        if(isContainDollarSign(str))
            return 3;
        else
            return 4;
    }
    else if(strcmp(firstWord, "info") == 0){
        return 5;
    }
    else if(strcmp(firstWord, "fake") == 0){
        return 6;
    }
    else if(strcmp(firstWord, "loan") == 0){
        return 7;
    }
    else if(strcmp(firstWord, "finish") == 0){
        return 8;
    }

    return -1;
}

int isContainDollarSign(char *str) {
    while (*str) {
        if (*str == '$'){
            return 1;
        }
        str++;
    }

    return 0;
}

int isAllFin(){
    int cnt=0;
    for(int i = 0; i < MAX_PLAYERS; i++)
        if(players[i].connfd != -1 && players[i].isFin == 0)
            return 0;
    return 1;
}
