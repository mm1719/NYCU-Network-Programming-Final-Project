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
    int loan_expense;
    int isFin; //bool
    int isDoneFake; //bool
    int isSetFake; //bool
    int isAchieved; //bool
    int total_loss; //only for character 3
} Player;

typedef struct{
    char news_content[300];
    char details_content[300];
    float fluctuations[8]; //example: {0.0, 0.0, 0.3, 0.0, -0.2, 0.0, 0.0, 0.0}
} News;

//Constant Global Variables
const int MAX_PLAYERS = 2;
const char WELCOME_AND_CHARACTERS[600] = 
"Welcome! Choose your character:\n\n"
"no.0: 老百姓\n"
"\t能力: 無\n"
"\t挑戰: 無\n"
"no.1: 交大富二代\n"
"\t能力: 無法向銀行借錢, 初始積分可多獲得1百萬\n" //done
"\t挑戰: 若全程只購買股票類商品, 獲得1百萬積分\n" //to do: 獲得1百萬積分
"no.2: 空軍上將\n" 
"\t能力: 做空可賺2倍獲利, 做多僅賺0.5倍獲利\n" //unimplemented
"\t挑戰: 若全程做空, 獲得1百萬積分\n" //to do: 獲得1百萬積分
"no.3: 虧損鬼才\n"
"\t能力: 最終結算時, 將2倍的虧損金額加至對手積分\n" //unimplemented
"\t挑戰: 若全程虧損且每次虧損金額不小於於現有積分之10%%, 獲得3百萬積分\n\n" //unimplemented
"請輸入整數(0 ~ 3)選擇角色:\n";
const int TOTAL_NEWS = 30; //TBD
const char NEWS_CONTENTS[TOTAL_NEWS][200] = { //TBD
    "AAA",
    "BBB"
};
const float NEWS_FLUCTUATIONS[TOTAL_NEWS][8] = { //TBD
    {0.0, 0.0, 0.3, 0.0, -0.2, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.2, 0.0}
};
const char ITEM_NAMES[8] = {"股票1", "股票2","鑽石"}; //TBD
const int ITEM_INIT_PRICE[8] = {1000, 10000}; //TBD
const int OP_INFO_PRICE = 200000; //TBD
const int OP_FAKE_PRICE = 500000; //TBD
const int LOAN_INTEREST = 20; //TBD

//Global Variables
int listenfd, maxfd;
fd_set allset;
Player players[MAX_PLAYERS]; //for saving active players' info
int active_players;
int current_round = 1;
News news_rounds[5][3], news_rounds_fake[5][3];
int item_prices_rounds[5][8];
int item_fluctuations_rounds[5][8];
int bankrupt_count;

//Functions
void handle_new();
void handle_in_round_msg(int);
int isAllFin();
News* get_3_random_news();
News* pick_1_fake_news(News*);
int extract_instr(char*);
int isContainDollarSign(char*);

int main(){
    //Socket settings
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
    news_rounds[0] = get_3_random_news(); //news for this round

    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 3; j++)
            for(int k = 0; k < 8; k++)
                item_fluctuations_rounds[0][i] += news_rounds[0][j].fluctuations[k]; //fluctuations for closing phase

    for(int i = 0; i < 8; i++){
        item_prices_rounds[0][i] = ITEM_INIT_PRICE[i]; //prices for this round
        item_prices_rounds[1][i] = item_prices_rounds[0][i] * item_fluctuations_rounds[0][i]; //prices for next round
    }

    for(int i = 0; i < MAX_PLAYERS; i++){ //send round and news
        if(players[i].connfd != -1){
            if(players[i].character == 1){
                players[i].points += 1000000; //init points - 交大富二代
                Writen(players[i].connfd, "Round 1\nYou have 2,000,000 points!\n", 35); //send round and points info
            }
            else
                Writen(players[i].connfd, "Round 1\nYou have 1,000,000 points!\n", 35); //send round and points info

            char buffer4[MAXLINE];
            sprintf(buffer4, "MARKET NEWS:\nNEWS 1: \t%s\nNEWS 2: \t%s\nNEWS 3: \t%s\n\n",
                    news_rounds[0].news_content, news_rounds[1].news_content, news_rounds[2].news_content);
            Writen(players[i].connfd, buffer4, strlen(buffer4)); //send market news
            memset(buffer4, 0, sizeof(buffer4)); //clear buffer4
        }
    }

    for(int i = 0; i < MAX_PLAYERS; i++){ //send prices
        if(players[i].connfd != -1){
            char buffer5[MAXLINE];
            sprintf(buffer5, "(1)%s:\t$%d\n(2)%s:\t$%d\n(3)%s:\t$%d\n(4)%s:\t$%d\n(5)%s:\t$%d\n(6)%s:\t$%d\n(7)%s:\t$%d\n(8)%s:\t$%d\n",
                    ITEM_NAMES[0], item_prices_rounds[0][0], ITEM_NAMES[1], item_prices_rounds[0][1], 
                    ITEM_NAMES[2], item_prices_rounds[0][2], ITEM_NAMES[3], item_prices_rounds[0][3], 
                    ITEM_NAMES[4], item_prices_rounds[0][4], ITEM_NAMES[5], item_prices_rounds[0][5], 
                    ITEM_NAMES[6], item_prices_rounds[0][6], ITEM_NAMES[7], item_prices_rounds[0][7]);
            Writen(players[i].connfd, buffer5, strlen(buffer5)); //send item prices
            memset(buffer5, 0, sizeof(buffer5)); //clear buffer5
        }
    }

    //Round 1 - player phase
    while (!isAllFin()) {
        fd_set rset = allset;
        Select(maxfd + 1, &rset, NULL, NULL, NULL);
        for (int i = 0; i < MAX_PLAYERS; i++){
            if (players[i].connfd != -1 && FD_ISSET(players[i].connfd, &rset)){
                handle_in_round_msg(i); //select and handle readable connfd
            }
        }
    }
    
    //Round 1 - closing phase
    for(int i = 0; i < MAX_PLAYERS; i++){
        if (players[i].connfd != -1){
            char buffer6[MAXLINE];
            sprintf(buffer6, "end of this round.\n, items fluctuations for this round is:\n");
            Writen(players[i].connfd, buffer6, strlen(buffer6)); //send text
            memset(buffer6, 0, sizeof(buffer6)); //clear buffer6

            for(int j = 0; j < 8; j++){
                if(item_fluctuations_rounds[0][j]){
                    char buffer7[MAXLINE];
                    sprintf(buffer7, "\t%s: \t%f\n");
                    Writen(players[i].connfd, buffer7, strlen(buffer7)); //send item fluctuations
                    memset(buffer7, 0, sizeof(buffer7)); //clear buffer7
                }
            }

            int earned = 0;
            for(int j = 0; j < 8; j++){
                earned += players[i].bought_amounts[j] * item_prices_rounds[1][j];
            }
            players[i].points += earned;

            char buffer8[MAXLINE];
            sprintf(buffer8, "your revenue on investment for this round is %d points.\nyou have %d point in total right now.\n", 
                    earned, players[i].points);
            Writen(players[i].connfd, buffer8, strlen(buffer8)); //send revenues and total points
            memset(buffer8, 0, sizeof(buffer8)); //clear buffer8

            if(players[i].loan_expense){
                players[i].points -= players[i].loan_expense; //update points

                char buffer9[MAXLINE];
                sprintf(buffer9, "your loan expense is $%d, the bank has deducted the points. your current points is $%d\n", 
                        players[i].loan_expense, players[i].points);
                Writen(players[i].connfd, buffer9, strlen(buffer9)); //send loan expense and the updated points
                memset(buffer9, 0, sizeof(buffer9)); //clear buffer9

                players[i].loan_expense = 0; //clear loan

                if(players[i].points <= 0){
                    bankrupt_count++;

                    char buffer10[MAXLINE];
                    sprintf(buffer10, "You have gone bankrupt, game over.\n", players[i].loan_expense, players[i].points);
                    Writen(players[i].connfd, buffer10, strlen(buffer10)); //send player bankrupt
                    memset(buffer10, 0, sizeof(buffer10)); //clear buffer10
                    players[i].connfd=-1; //remove this player

                    for(int j = 0; j < MAX_PLAYERS; j++){
                        if(players[j].connfd != -1 && i != j){
                            char buffer11[MAXLINE];
                            sprintf(buffer11, "player %s bankrupt.\n", players[i].name);
                            Writen(players[i].connfd, buffer11, strlen(buffer11)); //send who bankrupt
                            memset(buffer11, 0, sizeof(buffer11)); //clear buffer11
                        }
                    }

                    if(bankrupt_count >= MAX_PLAYERS - 1){
                        char buffer11[MAXLINE];
                        sprintf(buffer11, "You are the only one who has not gone bankrupt. ending the game...\n");
                        Writen(players[i].connfd, buffer11, strlen(buffer11)); //send who bankrupt
                        memset(buffer11, 0, sizeof(buffer11)); //clear buffer11
                        goto end_game;
                    }
                }
            }
        }
    }
    
    //Round 2
    current_round = 2;
    news_rounds[1] = get_3_random_news();
    news_rounds_fake[1] = pick_1_fake_news(news_rounds[1]);
    //copy from Round 1: 設定下回合的item_prices_rounds&item_fluctuations_rounds, 送出新聞, player phase, ending phase
    //new things to do: 送出假新聞if(players[i].isSetFake)

    //Round 3
    current_round = 3;
    news_rounds[2] = get_3_random_news();
    news_rounds_fake[2] = pick_1_fake_news(news_rounds[2]);
    //copy round 2

    //Round 4
    current_round = 4;
    news_rounds[3] = get_3_random_news();
    news_rounds_fake[3] = pick_1_fake_news(news_rounds[3]);
    //copy round 2

    //Round 5
    current_round = 5;
    news_rounds[4] = get_3_random_news();
    news_rounds_fake[4] = pick_1_fake_news(news_rounds[4]);
    //copy round 2 & 技能(虧損轉移)

    //After game
    //copy round 2
    //new things to do: 挑戰成功加分(完成一半: if(players[i].isAchieved==-1)character1&2不加分; =0則加分, 尚未實作: character3), 最終分數, 誰是贏家
    end_game://有可能是破產才來到end_game

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
            players[i].loan_expense = 0;
            players[i].isAllFin = 0;
            players[i].isDoneFake = 0;
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

            return;
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

        memset(players[player_i].name, 0, sizeof(players[player_i].name)); //clear player name
		//memset(players[player_i].ip, 0, sizeof(players[player_i].ip)); //clear player ip
    }

    //player instructions
	else {
		int instr = extract_instr(buffer3);

        char op_fail[MAXLINE];
        char op_done[MAXLINE];
        switch (instr){
            case 1: //long with $dollar
                int long_dollar_target, long_dollar_buy;
                sscanf(buffer3, "%*s %d $%d\n", &long_dollar_target, &long_dollar_buy);
                long_dollar_target--;

                if(long_dollar_buy > players[player_i].points){
                    sprintf(op_fail, "(insufficient points.)\n");
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else{
                    int long_dollar_bought_amount = long_dollar_buy / item_prices_rounds[current_round - 1][long_dollar_target];
                    int long_dollar_cost = item_prices_rounds[current_round - 1][long_dollar_target] * long_dollar_bought_amount;
                    players[player_i].points -= long_dollar_cost;
                    players[player_i].bought_amounts[long_dollar_target] += long_dollar_bought_amount;

                    sprintf(op_done, "(successfully long %d amounts of %s, costing %d points.)\n", 
                            long_dollar_bought_amount, ITEM_NAMES[long_dollar_target], long_dollar_cost);
                    Writen(players[player_i].connfd, op_done, strlen(op_done));
                }

                if(players[player_i].character == 1 && long_dollar_target != 0 && long_dollar_target != 1)
                    players[player_i].isAchieved = -1; //achievement failed - student buy non-stock items
                if(players[player_i].character == 2)
                    players[player_i].isAchieved = -1; //achievement failed - air force general goes long
                break;
            
            case 2: //long with amount
                int long_amount_target, long_amount_bought_amount;
                sscanf(buffer3, "%*s %d %d\n", &long_amount_target, &long_amount_bought_amount);
                long_amount_target--;

                int long_amount_cost = item_prices_rounds[current_round - 1][long_amount_target] * long_amount_bought_amount;
                if(long_amount_cost > players[player_i].points){
                    sprintf(op_fail, "(insufficient points, %d points required.)\n", long_amount_cost);
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else{
                    players[player_i].points -= long_amount_cost;
                    players[player_i].bought_amounts[long_amount_target] += long_amount_bought_amount;
                    
                    sprintf(op_done, "(successfully long %d amounts of %s, costing %d points.)\n", 
                            long_amount_bought_amount, ITEM_NAMES[long_amount_target], long_amount_cost);
                    Writen(players[player_i].connfd, op_done, strlen(op_done));
                }
                
                if(players[player_i].character == 1 && long_amount_target != 0 && long_amount_target != 1)
                    players[player_i].isAchieved = -1; //achievement failed - student buy non-stock items
                if(players[player_i].character == 2)
                    players[player_i].isAchieved = -1; //achievement failed - air force general goes long
                break;
            
            case 3: //short with $dollar
                int short_dollar_target, short_dollar_buy;
                sscanf(buffer3, "%*s %d $%d\n", &short_dollar_target, &short_dollar_buy);
                short_dollar_target--;

                if(short_dollar_buy > players[player_i].points){
                    sprintf(op_fail, "(insufficient points.)\n");
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else{
                    int short_dollar_bought_amount = short_dollar_buy / item_prices_rounds[current_round - 1][short_dollar_target];
                    int short_dollar_cost = item_prices_rounds[current_round - 1][short_dollar_target] * short_dollar_bought_amount;
                    players[player_i].points -= short_dollar_cost;
                    players[player_i].bought_amounts[short_dollar_target] -= short_dollar_bought_amount;

                    
                    sprintf(op_done, "(successfully short %d amounts of %s, costing %d points.)\n", 
                            short_dollar_bought_amount, ITEM_NAMES[short_dollar_target], short_dollar_cost);
                    Writen(players[player_i].connfd, op_done, strlen(op_done));
                }
                
                if(players[player_i].character == 1 && short_dollar_target != 0 && short_dollar_target != 1)
                    players[player_i].isAchieved = -1; //achievement failed - student buy non-stock items
                break;
            
            case 4: //short with amount
                int short_amount_target, short_amount_bought_amount;
                sscanf(buffer3, "%*s %d %d\n", &short_amount_target, &short_amount_bought_amount);
                short_amount_target--;

                int short_amount_cost = item_prices_rounds[current_round - 1][short_amount_target] * short_amount_bought_amount;
                if(short_amount_cost > players[player_i].points){
                    sprintf(op_fail, "(insufficient points, %d points required.)\n", short_amount_cost);
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else{
                    players[player_i].points -= short_amount_cost;
                    players[player_i].bought_amounts[short_amount_target] += short_amount_bought_amount;

                    sprintf(op_done, "(successfully short %d amounts of %s, costing %d points.)\n", 
                            short_amount_bought_amount, ITEM_NAMES[short_amount_target], short_amount_cost);
                    Writen(players[player_i].connfd, op_done, strlen(op_done));
                }
                
                if(players[player_i].character == 1 && short_amount_target != 0 && short_amount_target != 1)
                    players[player_i].isAchieved = -1; //achievement failed - student buy non-stock items
                break;
            
            case 5: //info
                if(players[player_i].points < OP_INFO_PRICE){
                    sprintf(op_fail, "(info operation requires %d points, you only have %d points.)\n", 
                            OP_INFO_PRICE, players[player_i].points);
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else{
                    players[player_i].points -= OP_INFO_PRICE;

                    int info_i;
                    sscanf(buffer3, "%*s %d", &info_i);

                    sprintf(op_done, "(details for NEWS %d: %s)\n", info_i, news_rounds[current_round][info_i - 1].details_content);
                    Writen(players[player_i].connfd, op_done, strlen(op_done));
                }
                break;
            
            case 6: //fake
                if(players[player_i].points < OP_FAKE_PRICE){
                    sprintf(op_fail, "(fake operation requires %d points, you only have %d points.)\n", 
                            OP_FAKE_PRICE, players[player_i].points);
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else if(current_round == 5){
                    sprintf(op_fail, "(you cannot do fake operation in the final round.)\n");
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else if(players[player_i].isDoneFake){
                    sprintf(op_fail, "(nice try, but you already faked your opponent's news this round, you nasty little weasel.)\n");
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else{
                    players[player_i].points -= OP_FAKE_PRICE;
                    players[player_i].isDoneFake = 1;
                    for (int i = 0; i < MAX_USERS; i++)
                        if (i != player_i && players[i].connfd != -1)
                            players[i].isSetFake = 1;

                    sprintf(op_done, "(a fake news will be deployed in your opponent's market news next round.)\n");
                    Writen(players[player_i].connfd, op_done, strlen(op_done));
                }
                break;
            
            case 7: //loan
                int need;
                sscanf(buffer3, "%*s %d\n", &need);

                if(players[player_i].character == 1){
                    sprintf(op_fail, "(no loan for students.)\n");
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else if(players[player_i].loan_expense){
                    sprintf(op_fail, "(nice try, but the bank already credited to you this round, you cheeky doughnut.)\n");
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else if(need > players[player_i].points){
                    sprintf(op_fail, "(your loan limit is $%d, which is your current points.)\n", players[player_i].points);
                    Writen(players[player_i].connfd, op_fail, strlen(op_fail));
                }
                else{
                    players[player_i].points += need;
                    players[player_i].loan_expense = need * (100 + LOAN_INTEREST) / 100;

                    sprintf(op_done, "(the bank credited $%d to you. you will need to pay $%d in the end of this round.)\n", 
                            need, players[player_i].loan_expense);
                    Writen(players[player_i].connfd, op_done, strlen(op_done));
                }
                
                break;
            
            case 8: //finish
                players[player_i].isFin = 1;
                break;
            
            // case -1: //invalid
            //     break;
        }

        memset(op_fail, 0, sizeof(op_fail));
        memset(op_done, 0, sizeof(op_done));
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

News* pick_1_fake_news(News* news_in){
    srand(time(0));
    int random = rand() % 3;

    News n[3];
    for(int i = 0; i < 3; i++){
        strncpy(n[i].news_content, news_in[i].news_content, strlen(news_in[i].news_content)); //save news content
        for (int j = 0; j < 8; j++) {
            n[i].fluctuations[j] = news_in[i].fluctuations; //save news fluctuations
        }
    }

    for (int i = 0; i < 8; j++)
        n[random].fluctuations = -n[random].fluctuations;

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
