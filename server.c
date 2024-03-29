// Libraries
#include "unp.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// use define instead of global constant for global range
#define TOTAL_NEWS 30
#define MAX_PLAYERS 2

// Structures
typedef struct
{
    int connfd;
    char name[100];
    // char ip[100];
    int character;
    int points;
    int bought_amounts[8]; // +,- : buy,sell
    int loan_expense;
    int isFin;      // bool
    int isDoneFake; // bool
    int isSetFake;  // bool
    int isAchieved; // bool
    int total_loss; // only for character 3
} Player;

typedef struct
{
    char news_content[300];
    char details_content[300];
    float fluctuations[8]; // example: {0.0, 0.0, 0.3, 0.0, -0.2, 0.0, 0.0, 0.0}
} News;

// Constant Global Variables
// const int MAX_PLAYERS = 2; //replaced with define
const char WELCOME_AND_CHARACTERS[600] =
    "Welcome! Choose your character:\n\n"
    "no.0: 老百姓\n"
    "\t能力: 無\n"
    "\t挑戰: 無\n"
    "no.1: 交大富二代\n"
    "\t能力: 無法向銀行借錢, 初始積分可多獲得1百萬\n" // done
    "\t挑戰: 若全程只購買股票類商品, 獲得1百萬積分\n" // to do: 獲得1百萬積分
    "no.2: 空軍上將\n"
    "\t能力: 做空可賺2倍獲利, 做多僅賺0.5倍獲利\n" // unimplemented
    "\t挑戰: 若全程做空, 獲得1百萬積分\n"          // to do: 獲得1百萬積分
    "no.3: 虧損鬼才\n"
    "\t能力: 最終結算時, 將2倍的虧損金額加至對手積分\n"                       // unimplemented
    "\t挑戰: 若全程虧損且每次虧損金額不小於現有積分之10%%, 獲得3百萬積分\n\n" // unimplemented
    "請輸入整數(0 ~ 3)選擇角色:\n";

// const int TOTAL_NEWS = 30; //replaced with define
const char NEWS_CONTENTS[TOTAL_NEWS][200] = {
    "麥當勞近期將在全球範圍開立更多分店, 市場關注其擴張計畫是否能提振業績",
    "長榮航空股票受油價波動影響, 投資者紛紛探討公司應對油價的策略",
    "近期雞肉市場穩健, 雞價有穩定下降的趨勢",
    "長榮航空宣布擴大國際航線",
    "近期GTA6即將開發完成, 近期將在PS5及PC上發售",
    "近期原神啟動小胖子引發熱議",
    "近期受到全球經濟不確定性的影響, 投資者紛紛轉向避險資產",
    "颱風肆虐, 農產品供給緊張",
    "國家經濟快速發展, 人民生活富足",
    "近期生質能成為全世界的熱門話題",
    "新型病毒肆虐, 家家戶戶不敢出門",
    "勞權意識抬頭, 世界各地三不五時出現罷工遊行",
    "疫情解封, 近期搜尋引擎的熱門字眼是出國旅遊",
    "蝗災過境, 許多農業大國身受其害",
    "任天堂最新廣告涉及種族歧視, 許多KOL紛紛出來抵制",
    "麥當勞薯條用油被專家踢爆離食安標準天差地遠",
    "冬天將至, 情侶們引頸期盼的12/14和12/25就要來了!",
    "最近駭客猖狂肆虐, 遊戲主機公司苦不堪言",
    "躺平現象普及全球, 年輕人寧可在家玩遊戲啃老",
    "分析師透漏, 下一季開始美元走向強勢, 央行勢必提升利率",
    "全球各地通膨嚴重, 抗通膨成了投資人的熱門話題",
    "暑假快到了, 世界各地飯店訂約爆滿, 旅遊業者大賺一筆",
    "俄烏戰爭引發供應風險, 中國近期集中採購美國農產品",
    "人權團體痛訴, 開採集團又開始偷偷雇用童工開採鑽石",
    "台灣鬼故事大師司馬中原, 於1/4撒手辭世了",        // 這則新聞不會造成任何影響
    "歐本海默的導演榮獲金球獎最佳導演的殊榮",         // 這則新聞不會造成任何影響
    "中國發射衛星飛越南台灣上空, 國防部緊急發布訊息", // 這則新聞不會造成任何影響
    "山道猴子迅速竄紅，青年紛紛效仿高速過彎，遊戲公司推出任天堂卡帶",
    "漢堡王宣布離開台灣並撤出所有業務",
    "OPEC宣布對支持以色列的國家實施制裁"};
const float NEWS_FLUCTUATIONS[TOTAL_NEWS][8] = {
    //"麥當勞股票","長榮航空股票","黃金","鑽石","石油","大豆","PS5遊戲卡帶","任天堂遊戲卡帶"
    {0.4, 0.0, 0.0, -0.1, 0.0, 0.0, 0.0, 0.0},
    {0.0, -0.3, 0.0, 0.0, 0.4, 0.0, 0.0, 0.0},
    {0.3, 0.0, 0.0, 0.0, 0.0, -0.3, 0.0, 0.0},
    {0.0, 0.3, 0.0, 0.0, 0.0, 0.0, -0.2, -0.2},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, -0.3},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.2, -0.2},
    {0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0},
    {-0.3, 0.0, 0.0, 0.0, 0.1, 0.4, 0.0, 0.0},
    {-0.2, 0.2, 0.0, 0.4, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, -0.3, 0.3, 0.0, 0.0},
    {0.0, -0.4, 0.0, 0.0, 0.0, 0.0, 0.2, 0.3},
    {-0.1, -0.2, 0.0, 0.0, 0.0, 0.0, -0.2, -0.1},
    {0.0, 0.3, 0.0, 0.0, 0.0, 0.0, -0.1, -0.2},
    {-0.1, 0.0, 0.0, 0.0, 0.0, -0.2, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.3, 0.1},
    {-0.3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.1, -0.1},
    {-0.2, -0.1, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1},
    {0.0, -0.2, -0.1, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.4, 0.0, 0.0, 0.0, 0.0, -0.3, -0.2},
    {0.0, 0.0, 0.0, 0.0, 0.2, 0.2, 0.0, 0.0},
    {0.0, 0.0, -0.3, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.1, 0.4},
    {0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.4, 0.0, 0.0, 0.0},
};
const char DETAILED_CONTENTS[TOTAL_NEWS][200] = {
    "受影響: 麥當勞股票, 鑽石等",
    "受影響: 長榮航空股票, 石油等",
    "受影響: 麥當勞股票, 大豆等",
    "受影響: 長榮航空股票, PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 黃金等",
    "受影響: 麥當勞股票, 石油, 大豆等",
    "受影響: 麥當勞股票, 長榮航空股票, 鑽石等",
    "受影響: 石油, 大豆等",
    "受影響: 長榮航空股票, PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 麥當勞股票, 長榮航空股票, PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 長榮航空股票, PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 麥當勞股票, 大豆等",
    "受影響: PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 麥當勞股票等",
    "受影響: 鑽石等",
    "受影響: PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 麥當勞股票, 長榮航空股票, PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 長榮航空股票, 黃金等",
    "受影響: 黃金等",
    "受影響: 長榮航空股票, PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 石油, 大豆等",
    "受影響: 鑽石等",
    "受影響: 無", // 這則新聞不會造成任何影響
    "受影響: 無", // 這則新聞不會造成任何影響
    "受影響: 無", // 這則新聞不會造成任何影響
    "受影響: PS5遊戲卡帶, 任天堂遊戲卡帶等",
    "受影響: 麥當勞股票等",
    "受影響: 石油等",
};
const char *ITEM_NAMES[8] = {"麥當勞股票", "長榮航空股票", "黃金", "鑽石", "石油", "大豆", "PS5遊戲卡帶", "任天堂遊戲卡帶"};
const int ITEM_INIT_PRICE[8] = {98, 128, 666, 1010, 504, 99, 159, 137};
const int OP_INFO_PRICE = 200000; // 購買 info 的價格
const int OP_FAKE_PRICE = 500000; // 購買 fake news 的價格
const int LOAN_INTEREST = 20;     // 借貸利率

// Global Variables
int listenfd, maxfd;
fd_set allset, rset;
Player players[MAX_PLAYERS]; // for saving active players' info
int active_players = 0;
int current_round = 1;
News news_rounds[5][3];
float item_fluctuation_rounds_fake[5][8];
int item_prices_rounds[5][8];
float item_fluctuations_rounds[5][8];
int bankrupt_count = 0;
// News* fakeNews;
int prevPoints[2];
char bigBuffer[2][MAXLINE] = {"", ""};

// Functions
void wait_players();
void handle_in_round_msg(int);
int isAllFin();
void get_3_random_news(News *);
// News* pick_1_fake_news(News*);
int extract_instr(char *);
int isContainDollarSign(char *);
void setRandomNews(int);
void setPriceAndFluctuations(int);
void sendPointsAndNews(int);
void sendPricesInfo(int);
void playerPhase();
void setFakeResult(int);
void closingPhase(int);
void claimWinner();

int main()
{
    srand(time(NULL));
    // Socket settings
    listenfd = Socket(AF_INET, SOCK_STREAM, 0); // ask for socket

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));           // clear socket
    servaddr.sin_family = AF_INET;                // socket settings - IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // socket settings - address
    servaddr.sin_port = htons(SERV_PORT + 5);     // socket settings - port

    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr)); // bind
    Listen(listenfd, LISTENQ);                         // listen

    for (int i = 0; i < MAX_PLAYERS; i++)
        players[i].connfd = -1; // no active player in the beginning

    maxfd = listenfd;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // Before game
    wait_players(); // wait for players
    fprintf(stdout, "Game started.\n");
    // Round 1 - prepare phase

    setRandomNews(current_round); // set random news for round 1

    setPriceAndFluctuations(current_round); // calculate item fluctuations for round 1
    /*for(int i = 0; i < 8; i++)
        for(int j = 0; j < 3; j++)
            for(int k = 0; k < 8; k++)
                item_fluctuations_rounds[0][i] += news_rounds[0][j].fluctuations[k]; //fluctuations for closing phase
    */

    /*for(int i = 0; i < 8; i++){
        item_prices_rounds[0][i] = ITEM_INIT_PRICE[i]; //prices for this round
        item_prices_rounds[1][i] = item_prices_rounds[0][i] * (1 + item_fluctuations_rounds[0][i]); //prices for next round
    }*/

    sendPointsAndNews(current_round); // send points and news in round 1
    /*
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
                    news_rounds[0]->news_content, news_rounds[1]->news_content, news_rounds[2]->news_content);
            Writen(players[i].connfd, buffer4, strlen(buffer4)); //send market news
            memset(buffer4, 0, sizeof(buffer4)); //clear buffer4
        }
    }
    */

    sendPricesInfo(current_round); // send round 1 info
    /*
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
    */

    // Round 1 - player phase
    playerPhase(); // wait for players to finish their operations in round 1
    /*while (!isAllFin()) {
        fd_set rset = allset;
        Select(maxfd + 1, &rset, NULL, NULL, NULL);
        for (int i = 0; i < MAX_PLAYERS; i++){
            if (players[i].connfd != -1 && FD_ISSET(players[i].connfd, &rset)){
                handle_in_round_msg(i); //select and handle readable connfd
            }
        }
    }*/

    if (players[0].isSetFake)
    { // 送出假新聞if(players[0].isSetFake)
        setFakeResult(current_round);
        players[0].isDoneFake = 0;
        players[0].isSetFake = 0;
    }
    if (players[1].isSetFake)
    { // 送出假新聞if(players[1].isSetFake)
        setFakeResult(current_round);
        players[1].isDoneFake = 0;
        players[1].isSetFake = 0;
    }
    printf("closing phase\n");
    memset(bigBuffer[0], 0, sizeof(bigBuffer[0]));
    memset(bigBuffer[1], 0, sizeof(bigBuffer[1]));
    // Round 1 - closing phase
    closingPhase(current_round); // settle the account in round 1
    checkCharacter3();           // check if character3 fails
    if (bankrupt_count >= MAX_PLAYERS - 1)
        goto end_game;

    /*for(int i = 0; i < MAX_PLAYERS; i++){
        if (players[i].connfd != -1){
            char buffer6[MAXLINE];
            sprintf(buffer6, "end of this round.\n, items fluctuations for this round is:\n");
            Writen(players[i].connfd, buffer6, strlen(buffer6)); //send text
            memset(buffer6, 0, sizeof(buffer6)); //clear buffer6

            for(int j = 0; j < 8; j++){
                if(item_fluctuations_rounds[0][j]){
                    char buffer7[MAXLINE];
                    sprintf(buffer7, "\t%s: \t%f\n", ITEM_NAMES[j], item_fluctuations_rounds[0][j]);
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
    }*/

    // Round 2
    current_round = 2;
    prevPoints[0] = players[0].points;
    prevPoints[1] = players[1].points;
    setRandomNews(current_round);           // set random news for round 2
    setPriceAndFluctuations(current_round); // calculate item fluctuations for round 2
    sendPointsAndNews(current_round);       // send points and news in round 2
    sendPricesInfo(current_round);          // send round 2 info
    playerPhase();                          // wait for players to finish their operations in round 2
    if (players[0].isSetFake)
    { // 送出假新聞if(players[0].isSetFake)
        setFakeResult(current_round);
        players[0].isDoneFake = 0;
        players[0].isSetFake = 0;
    }
    if (players[1].isSetFake)
    { // 送出假新聞if(players[1].isSetFake)
        setFakeResult(current_round);
        players[1].isDoneFake = 0;
        players[1].isSetFake = 0;
    }
    closingPhase(current_round); // settle the account in round 2
    checkCharacter3();           // check if character3 fails
    if (bankrupt_count >= MAX_PLAYERS - 1)
        goto end_game;

    // Round 3
    current_round = 3;
    prevPoints[0] = players[0].points;
    prevPoints[1] = players[1].points;
    setRandomNews(current_round);           // set random news for round 3
    setPriceAndFluctuations(current_round); // calculate item fluctuations for round 3
    sendPointsAndNews(current_round);       // send points and news in round 3
    sendPricesInfo(current_round);          // send round 3 info
    playerPhase();                          // wait for players to finish their operations in round 3
    if (players[0].isSetFake)
    { // 送出假新聞if(players[0].isSetFake)
        setFakeResult(current_round);
        players[0].isDoneFake = 0;
        players[0].isSetFake = 0;
    }
    if (players[1].isSetFake)
    { // 送出假新聞if(players[1].isSetFake)
        setFakeResult(current_round);
        players[1].isDoneFake = 0;
        players[1].isSetFake = 0;
    }
    closingPhase(current_round); // settle the account in round 3
    checkCharacter3();           // check if character3 fails
    if (bankrupt_count >= MAX_PLAYERS - 1)
        goto end_game;

    // Round 4
    current_round = 4;
    prevPoints[0] = players[0].points;
    prevPoints[1] = players[1].points;
    setRandomNews(current_round);           // set random news for round 4
    setPriceAndFluctuations(current_round); // calculate item fluctuations for round 4
    sendPointsAndNews(current_round);       // send points and news in round 4
    sendPricesInfo(current_round);          // send round 4 info
    playerPhase();                          // wait for players to finish their operations in round 4
    if (players[0].isSetFake)
    { // 送出假新聞if(players[0].isSetFake)
        setFakeResult(current_round);
        players[0].isDoneFake = 0;
        players[0].isSetFake = 0;
    }
    if (players[1].isSetFake)
    { // 送出假新聞if(players[1].isSetFake)
        setFakeResult(current_round);
        players[1].isDoneFake = 0;
        players[1].isSetFake = 0;
    }
    closingPhase(current_round); // settle the account in round 4
    checkCharacter3();           // check if character3 fails
    if (bankrupt_count >= MAX_PLAYERS - 1)
        goto end_game;

    // Round 5
    current_round = 5;
    prevPoints[0] = players[0].points;
    prevPoints[1] = players[1].points;
    setRandomNews(current_round);           // set random news for round 5
    setPriceAndFluctuations(current_round); // calculate item fluctuations for round 5
    sendPointsAndNews(current_round);       // send points and news in round 5
    sendPricesInfo(current_round);          // send round 5 info
    playerPhase();                          // wait for players to finish their operations in round 5
    closingPhase(current_round);            // settle the account in round 5
    checkCharacter3();                      // check if character3 fails
    if (bankrupt_count >= MAX_PLAYERS - 1)
        goto end_game;

    // character3技能(虧損轉移)
    int loss[2];
    for (int i = 0, j = 1; i < 2; i++, j--)
    {
        if (players[i].character == 3)
            loss[i] = (1000000 - players[i].points) * 2;
    }
    for (int i = 0, j = 1; i < 2; i++, j--)
    {
        if (players[i].character == 3)
            if (loss[i] > 0)
                players[j].points -= loss[i];
    }

    // After game
    // 挑戰成功加分(完成一半: 最終分數, 誰是贏家

    for (int i = 0; i < 2; i++)
    {
        if (players[i].isAchieved == 0)
        {
            if (players[i].character == 1 || players[i].character == 2)
                players[i].points += 1000000;
            else if (players[i].character == 3)
                players[i].points += 3000000;
        }
    }

    end_game: // 有可能是破產才來到end_game
    claimWinner();

    return 0;
}

void wait_players()
{
    int flag0 = 0, flag1 = 0;
    for (;;)
    {
        fprintf(stdout, "active_players: %d\n", active_players);

        if (active_players >= MAX_PLAYERS && flag0 && flag1)
        {
            fprintf(stdout, "game start!\n");
            break;
        }

        rset = allset;
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset))
        {
            if (active_players < 2)
            {
                int playerID = active_players;
                int connfd;
                struct sockaddr_in cliaddr;
                socklen_t clilen = sizeof(cliaddr);
                connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen); // accept
                active_players++;                                                // add new active player

                char buffer1[MAXLINE];
                ssize_t tmp = Read(connfd, buffer1, MAXLINE); // read name
                buffer1[tmp] = 0;                             // string ending

                if (players[playerID].connfd == -1)
                { // find an empty players slot
                    // init player info
                    players[playerID].connfd = connfd;
                    strncpy(players[playerID].name, buffer1, strlen(buffer1));
                    // strncpy(players[i].ip, inet_ntoa(cliaddr.sin_addr), strlen(inet_ntoa(cliaddr.sin_addr)));
                    players[playerID].points = 1000000;
                    players[playerID].loan_expense = 0;
                    players[playerID].isFin = 0;
                    players[playerID].isDoneFake = 0;
                    players[playerID].isSetFake = 0;
                    players[playerID].isAchieved = 0;
                    memset(players[playerID].bought_amounts, 0, sizeof(players[playerID].bought_amounts));

                    FD_SET(connfd, &allset);    // add this connfd to allset
                    maxfd = max(maxfd, connfd); // update maxfd

                    Writen(players[playerID].connfd, WELCOME_AND_CHARACTERS, strlen(WELCOME_AND_CHARACTERS)); // send character list

                    memset(buffer1, 0, sizeof(buffer1)); // clear buffer1

                    /*
                    char buffer2[MAXLINE];
                    int tmp2 = Read(connfd, buffer2, sizeof(buffer2)); //read character
                    fprintf(stdout, "%s\n", buffer2);
                    players[i].character = buffer2[0] - '0'; //save character

                    memset(buffer1, 0, sizeof(buffer1)); //clear buffer1
                    memset(buffer2, 0, sizeof(buffer2)); //clear buffer2

                    FD_SET(connfd, &allset); //add this connfd to allset
                    maxfd = max(maxfd, connfd); //update maxfd

                    return;
                    */
                }
            }
        }

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (players[i].connfd != -1 && FD_ISSET(players[i].connfd, &rset))
            {
                char buffer2[MAXLINE];
                ssize_t tmp2 = Read(players[i].connfd, buffer2, sizeof(buffer2));
                buffer2[tmp2] = 0;
                // fprintf(stdout, "%s\n", buffer2);
                players[i].character = buffer2[0] - '0';
                memset(buffer2, 0, sizeof(buffer2)); // clear buffer2
                if (i == 0)
                    flag0 = 1;
                else if (i == 1)
                    flag1 = 1;
            }
        }
    }
}

void handle_in_round_msg(int player_i)
{
    char buffer3[MAXLINE];
    int nbytes = Read(players[player_i].connfd, buffer3, MAXLINE);
    buffer3[nbytes] = 0;

    // player leaving
    if (nbytes <= 0)
    {
        active_players--;

        if (nbytes == 0)
            Writen(players[player_i].connfd, "Bye!\n", 5); // leave by Ctrl+D

        Close(players[player_i].connfd);
        FD_CLR(players[player_i].connfd, &allset);
        players[player_i].connfd = -1;

        char leaving[MAXLINE];
        if (active_players == 1)
        {
            sprintf(leaving, "(%s left. You win!)\n", players[player_i].name); // prepare leaving msg
            for (int i = 0; i < MAX_PLAYERS; i++)
                if (players[i].connfd != -1)
                    Writen(players[i].connfd, leaving, strlen(leaving)); // send leaving msg to last player
            memset(leaving, 0, sizeof(leaving));                         // clear leaving
        }
        else
        {
            sprintf(leaving, "(%s left, %d players left)\n", players[player_i].name, active_players); // prepare leaving msg
            for (int i = 0; i < MAX_PLAYERS; i++)
                if (players[i].connfd != -1)
                    Writen(players[i].connfd, leaving, strlen(leaving)); // send leaving msg to active players
            memset(leaving, 0, sizeof(leaving));                         // clear leaving
        }

        memset(players[player_i].name, 0, sizeof(players[player_i].name)); // clear player name
        // memset(players[player_i].ip, 0, sizeof(players[player_i].ip)); //clear player ip
    }

    // player instructions
    else
    {
        int instr = extract_instr(buffer3);

        char op_fail[MAXLINE];
        char op_done[MAXLINE];
        switch (instr)
        {
        case 1: // get long with $dollar
            ;
            int long_dollar_target, long_dollar_buy;
            sscanf(buffer3, "%*s %d $%d\n", &long_dollar_target, &long_dollar_buy);
            long_dollar_target--;

            if (long_dollar_buy > players[player_i].points)
            {
                sprintf(op_fail, "(insufficient points.)\n");
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else
            {
                int long_dollar_bought_amount = long_dollar_buy / item_prices_rounds[current_round - 1][long_dollar_target];
                int long_dollar_cost = item_prices_rounds[current_round - 1][long_dollar_target] * long_dollar_bought_amount;
                players[player_i].points -= long_dollar_cost;
                players[player_i].bought_amounts[long_dollar_target] += long_dollar_bought_amount;

                sprintf(op_done, "(successfully long %d amounts of %s, costing %d points.)\n",
                        long_dollar_bought_amount, ITEM_NAMES[long_dollar_target], long_dollar_cost);
                Writen(players[player_i].connfd, op_done, strlen(op_done));
            }

            if (players[player_i].character == 1 && long_dollar_target != 0 && long_dollar_target != 1)
                players[player_i].isAchieved = -1; // achievement failed - student buy non-stock items
            if (players[player_i].character == 2)
                players[player_i].isAchieved = -1; // achievement failed - air force general goes long
            break;

        case 2: // get long with amount
            ;
            int long_amount_target, long_amount_bought_amount;
            sscanf(buffer3, "%*s %d %d\n", &long_amount_target, &long_amount_bought_amount);
            long_amount_target--;

            int long_amount_cost = item_prices_rounds[current_round - 1][long_amount_target] * long_amount_bought_amount;
            if (long_amount_cost > players[player_i].points)
            {
                sprintf(op_fail, "(insufficient points, %d points required.)\n", long_amount_cost);
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else
            {
                players[player_i].points -= long_amount_cost;
                players[player_i].bought_amounts[long_amount_target] += long_amount_bought_amount;

                sprintf(op_done, "(successfully long %d amounts of %s, costing %d points.)\n",
                        long_amount_bought_amount, ITEM_NAMES[long_amount_target], long_amount_cost);
                Writen(players[player_i].connfd, op_done, strlen(op_done));
            }

            if (players[player_i].character == 1 && long_amount_target != 0 && long_amount_target != 1)
                players[player_i].isAchieved = -1; // achievement failed - student buy non-stock items
            if (players[player_i].character == 2)
                players[player_i].isAchieved = -1; // achievement failed - air force general goes long
            break;

        case 3: // get short with $dollar
            ;
            int short_dollar_target, short_dollar_buy;
            sscanf(buffer3, "%*s %d $%d\n", &short_dollar_target, &short_dollar_buy);
            short_dollar_target--;

            if (short_dollar_buy > players[player_i].points)
            {
                sprintf(op_fail, "(insufficient points.)\n");
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else
            {
                int short_dollar_bought_amount = short_dollar_buy / item_prices_rounds[current_round - 1][short_dollar_target];
                int short_dollar_cost = item_prices_rounds[current_round - 1][short_dollar_target] * short_dollar_bought_amount;
                players[player_i].points -= short_dollar_cost;
                players[player_i].bought_amounts[short_dollar_target] -= short_dollar_bought_amount;

                sprintf(op_done, "(successfully short %d amounts of %s, costing %d points.)\n",
                        short_dollar_bought_amount, ITEM_NAMES[short_dollar_target], short_dollar_cost);
                Writen(players[player_i].connfd, op_done, strlen(op_done));
            }

            if (players[player_i].character == 1 && short_dollar_target != 0 && short_dollar_target != 1)
                players[player_i].isAchieved = -1; // achievement failed - student buy non-stock items
            break;

        case 4: // get short with amount
            ;
            int short_amount_target, short_amount_bought_amount;
            sscanf(buffer3, "%*s %d %d\n", &short_amount_target, &short_amount_bought_amount);
            short_amount_target--;

            int short_amount_cost = item_prices_rounds[current_round - 1][short_amount_target] * short_amount_bought_amount;
            if (short_amount_cost > players[player_i].points)
            {
                sprintf(op_fail, "(insufficient points, %d points required.)\n", short_amount_cost);
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else
            {
                players[player_i].points -= short_amount_cost;
                players[player_i].bought_amounts[short_amount_target] += short_amount_bought_amount;

                sprintf(op_done, "(successfully short %d amounts of %s, costing %d points.)\n",
                        short_amount_bought_amount, ITEM_NAMES[short_amount_target], short_amount_cost);
                Writen(players[player_i].connfd, op_done, strlen(op_done));
            }

            if (players[player_i].character == 1 && short_amount_target != 0 && short_amount_target != 1)
                players[player_i].isAchieved = -1; // achievement failed - student buy non-stock items
            break;

        case 5: // info
            ;
            printf("In Info.\n");
            if (players[player_i].points < OP_INFO_PRICE)
            {
                sprintf(op_fail, "(info operation requires %d points, you only have %d points.)\n",
                        OP_INFO_PRICE, players[player_i].points);
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else
            {
                players[player_i].points -= OP_INFO_PRICE;

                int info_i;
                sscanf(buffer3, "%*s %d", &info_i);

                sprintf(op_done, "(details for NEWS %d: %s)\n", info_i, news_rounds[current_round][info_i - 1].details_content);
                Writen(players[player_i].connfd, op_done, strlen(op_done));
            }
            break;

        case 6: // fake
            ;
            if (players[player_i].points < OP_FAKE_PRICE)
            {
                sprintf(op_fail, "(fake operation requires %d points, you only have %d points.)\n",
                        OP_FAKE_PRICE, players[player_i].points);
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else if (current_round == 5)
            {
                sprintf(op_fail, "(you cannot do fake operation in the final round.)\n");
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else if (players[player_i].isDoneFake)
            {
                sprintf(op_fail, "(nice try, but you already faked your opponent's news this round, you nasty little weasel.)\n");
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else
            {
                players[player_i].points -= OP_FAKE_PRICE;
                players[player_i].isDoneFake = 1;
                for (int i = 0; i < MAX_PLAYERS; i++)
                    if (i != player_i && players[i].connfd != -1)
                        players[i].isSetFake = 1;

                sprintf(op_done, "(a fake news will be deployed in your opponent's market news next round.)\n");
                Writen(players[player_i].connfd, op_done, strlen(op_done));
            }
            break;

        case 7: // loan
            ;
            int need;
            sscanf(buffer3, "%*s %d\n", &need);

            if (players[player_i].character == 1)
            {
                sprintf(op_fail, "(no loan for students.)\n");
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else if (players[player_i].loan_expense)
            {
                sprintf(op_fail, "(nice try, but the bank already credited to you this round, you cheeky doughnut.)\n");
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else if (need > players[player_i].points)
            {
                sprintf(op_fail, "(your loan limit is $%d, which is your current points.)\n", players[player_i].points);
                Writen(players[player_i].connfd, op_fail, strlen(op_fail));
            }
            else
            {
                players[player_i].points += need;
                players[player_i].loan_expense = need * (100 + LOAN_INTEREST) / 100;

                sprintf(op_done, "(the bank credited $%d to you. you will need to pay $%d in the end of this round.)\n",
                        need, players[player_i].loan_expense);
                Writen(players[player_i].connfd, op_done, strlen(op_done));
            }

            break;

        case 8: // finish
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

void get_3_random_news(News *n)
{
    int rand_i[3]; // for randomly picking news

    do
    {
        for (int i = 0; i < 3; i++)
            rand_i[i] = rand() % TOTAL_NEWS;
    } while (rand_i[0] == rand_i[1] || rand_i[1] == rand_i[2] || rand_i[0] == rand_i[2]);

    printf("%d %d %d\n", rand_i[0], rand_i[1], rand_i[2]);

    for (int i = 0; i < 3; i++)
    {
        strncpy(n[i].news_content, NEWS_CONTENTS[rand_i[i]], strlen(NEWS_CONTENTS[rand_i[i]])); // save news content
        for (int j = 0; j < 8; j++)
            n[i].fluctuations[j] = NEWS_FLUCTUATIONS[rand_i[i]][j];                                        // save news fluctuations
        strncpy(n[i].details_content, DETAILED_CONTENTS[rand_i[i]], strlen(DETAILED_CONTENTS[rand_i[i]])); // save detailed content
    }
}

/*News* pick_1_fake_news(News* news_in){
    srand(time(0));
    int random = rand() % 3;

    News n[3];
    for(int i = 0; i < 3; i++){
        strncpy(n[i].news_content, news_in[i].news_content, strlen(news_in[i].news_content)); //save news content
        for (int j = 0; j < 8; j++) {
            n[i].fluctuations[j] = news_in[i].fluctuations; //save news fluctuations
        }
    }

    for (int i = 0; i < 8; i++)
        n[random].fluctuations = -n[random].fluctuations;

    return n;
}*/

int extract_instr(char *str)
{
    char firstWord[10];

    int i = 0;
    while (str[i] != ' ' && str[i] != '\n')
    {
        firstWord[i] = str[i];
        i++;
    }
    firstWord[i] = 0;

    if (strcmp(firstWord, "long") == 0)
    {
        if (isContainDollarSign(str))
            return 1;
        else
            return 2;
    }
    else if (strcmp(firstWord, "short") == 0)
    {
        if (isContainDollarSign(str))
            return 3;
        else
            return 4;
    }
    else if (strcmp(firstWord, "info") == 0)
    {
        return 5;
    }
    else if (strcmp(firstWord, "fake") == 0)
    {
        return 6;
    }
    else if (strcmp(firstWord, "loan") == 0)
    {
        return 7;
    }
    else if (strcmp(firstWord, "finish") == 0)
    {
        return 8;
    }

    return -1;
}

int isContainDollarSign(char *str)
{
    while (*str)
    {
        if (*str == '$')
        {
            return 1;
        }
        str++;
    }

    return 0;
}

int isAllFin()
{
    int cnt = 0;
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (players[i].connfd != -1 && players[i].isFin == 0)
            return 0;
    return 1;
}

void setRandomNews(int round)
{
    round--;
    fprintf(stdout, "Random News for round %d:\n", round + 1);

    News newNews[3];
    get_3_random_news(newNews); // The array is filled by the function
    fprintf(stdout, "%s\n", newNews[1].news_content);

    for (int i = 0; i < 3; i++)
    {
        news_rounds[round][i] = newNews[i];
    }
    fprintf(stdout, "%s\n", news_rounds[round][1].news_content);
}

void setPriceAndFluctuations(int round)
{
    round--;
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 3; j++)
            item_fluctuations_rounds[round][i] += news_rounds[round][j].fluctuations[i];

    if (round == 0)
        for (int i = 0; i < 8; i++)
            item_prices_rounds[0][i] = ITEM_INIT_PRICE[i]; // prices for this round

    for (int i = 0; i < 8; i++)
        item_prices_rounds[round + 1][i] = item_prices_rounds[round][i] * (1 + item_fluctuations_rounds[round][i]); // prices for next round
}

void sendPointsAndNews(int round)
{
    round--;
    for (int i = 0; i < MAX_PLAYERS; i++)
    { // send round and news
        if (players[i].connfd != -1)
        {
            if (round == 0)
            {
                if (players[i].character == 1)
                {
                    players[i].points += 1000000;                                           // init points - 交大富二代
                    Writen(players[i].connfd, "Round 1\nYou have 2,000,000 points!\n", 35); // send round and points info
                }
                else
                    Writen(players[i].connfd, "Round 1\nYou have 1,000,000 points!\n", 35); // send round and points info
            }
            else
            {
                char buffer12[MAXLINE];
                sprintf(buffer12, "Round %d\nYou have %d points!\n", round + 1, players[i].points);
                Writen(players[i].connfd, buffer12, strlen(buffer12)); // send round and points info
            }

            char buffer4[MAXLINE];
            sprintf(buffer4, "MARKET NEWS:\nNEWS 1: \t%s\nNEWS 2: \t%s\nNEWS 3: \t%s\n",
                    news_rounds[round][0].news_content, news_rounds[round][1].news_content, news_rounds[round][2].news_content);
            strcat(bigBuffer[i], buffer4);
            // Writen(players[i].connfd, buffer4, strlen(buffer4)); //send market news
            memset(buffer4, 0, sizeof(buffer4)); // clear buffer4
        }
    }
}

void sendPricesInfo(int round)
{
    round--;
    for (int i = 0; i < MAX_PLAYERS; i++)
    { // send prices
        if (players[i].connfd != -1)
        {
            char buffer5[MAXLINE];
            sprintf(buffer5, "(1)%s:\t\t$%d\n(2)%s:\t$%d\n(3)%s:\t\t$%d\n(4)%s:\t\t$%d\n(5)%s:\t\t$%d\n(6)%s:\t\t$%d\n(7)%s:\t\t$%d\n(8)%s:\t$%d\n",
                    ITEM_NAMES[0], item_prices_rounds[round][0], ITEM_NAMES[1], item_prices_rounds[round][1],
                    ITEM_NAMES[2], item_prices_rounds[round][2], ITEM_NAMES[3], item_prices_rounds[round][3],
                    ITEM_NAMES[4], item_prices_rounds[round][4], ITEM_NAMES[5], item_prices_rounds[round][5],
                    ITEM_NAMES[6], item_prices_rounds[round][6], ITEM_NAMES[7], item_prices_rounds[round][7]);

            // fprintf(stdout, "%s\n", buffer5);
            strcat(bigBuffer[i], buffer5);
            Writen(players[i].connfd, bigBuffer[i], strlen(bigBuffer[i]));
            // Writen(players[i].connfd, buffer5, strlen(buffer5)); // send item prices
            memset(buffer5, 0, sizeof(buffer5)); // clear buffer5
        }
    }
}

void playerPhase()
{
    while (!isAllFin())
    {
        fd_set rset = allset;
        Select(maxfd + 1, &rset, NULL, NULL, NULL);
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (players[i].connfd != -1 && FD_ISSET(players[i].connfd, &rset))
            {
                handle_in_round_msg(i); // select and handle readable connfd
            }
        }
    }
}

void closingPhase(int round)
{
    players[0].isFin = 0;
    players[1].isFin = 0;
    printf("In closing phase function.\n");
    round--;
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (players[i].connfd != -1)
        {
            char buffer6[MAXLINE];
            sprintf(buffer6, "end of this round.\n, items fluctuations for this round is:\n");
            Writen(players[i].connfd, buffer6, strlen(buffer6)); // send text
            memset(buffer6, 0, sizeof(buffer6));                 // clear buffer6

            for (int j = 0; j < 8; j++)
            {
                if (item_fluctuations_rounds[round][j])
                {
                    char buffer7[MAXLINE];
                    sprintf(buffer7, "\t%s: \t+%f%%\n", ITEM_NAMES[j], 100 * item_fluctuations_rounds[round][j]);
                    Writen(players[i].connfd, buffer7, strlen(buffer7)); // send item fluctuations
                    memset(buffer7, 0, sizeof(buffer7));                 // clear buffer7
                }
            }

            int earned = 0;
            for (int j = 0; j < 8; j++)
            {
                if (players[i].character == 2)
                {
                    if (players[i].bought_amounts[j] < 0)
                    {
                        earned += players[i].bought_amounts[j] * item_prices_rounds[round + 1][j] * 2;
                    }
                    else
                    {
                        earned += players[i].bought_amounts[j] * item_prices_rounds[round + 1][j] * 0.5;
                    }
                }
                else
                {
                    earned += players[i].bought_amounts[j] * item_prices_rounds[round + 1][j];
                }
            }
            players[i].points += earned;

            char buffer8[MAXLINE];
            sprintf(buffer8, "your revenue on investment for this round is %d points.\nyou have %d point in total right now.\n",
                    earned, players[i].points);
            Writen(players[i].connfd, buffer8, strlen(buffer8)); // send revenues and total points
            memset(buffer8, 0, sizeof(buffer8));                 // clear buffer8

            if (players[i].loan_expense)
            {
                players[i].points -= players[i].loan_expense; // update points

                char buffer9[MAXLINE];
                sprintf(buffer9, "your loan expense is $%d, the bank has deducted the points. your current points is $%d\n",
                        players[i].loan_expense, players[i].points);
                Writen(players[i].connfd, buffer9, strlen(buffer9)); // send loan expense and the updated points
                memset(buffer9, 0, sizeof(buffer9));                 // clear buffer9

                players[i].loan_expense = 0; // clear loan
            }

            for (int j = 0; j < 8; j++)
            {
                players[i].bought_amounts[j] = 0; // clear bought amounts
            }

            if (players[i].points <= 0)
            {
                bankrupt_count++;

                char buffer10[MAXLINE];
                sprintf(buffer10, "You have gone bankrupt, game over.\n", players[i].loan_expense, players[i].points);
                Writen(players[i].connfd, buffer10, strlen(buffer10)); // send player bankrupt
                memset(buffer10, 0, sizeof(buffer10));                 // clear buffer10
                players[i].connfd = -1;                                // remove this player

                for (int j = 0; j < MAX_PLAYERS; j++)
                {
                    if (players[j].connfd != -1 && i != j)
                    {
                        char buffer11[MAXLINE];
                        sprintf(buffer11, "player %s bankrupt.\n", players[i].name);
                        Writen(players[i].connfd, buffer11, strlen(buffer11)); // send who bankrupt
                        memset(buffer11, 0, sizeof(buffer11));                 // clear buffer11
                    }
                }

                if (bankrupt_count >= MAX_PLAYERS - 1)
                {
                    char buffer11[MAXLINE];
                    sprintf(buffer11, "You are the only one who has not gone bankrupt. ending the game...\n");
                    Writen(players[i].connfd, buffer11, strlen(buffer11)); // send who bankrupt
                    memset(buffer11, 0, sizeof(buffer11));                 // clear buffer11
                    break;
                }
            }
        }
    }
}

void setFakeResult(int round)
{
    round--;
    srand(time(0));
    int random = rand() % 3;
    for (int i = 0; i < 8; i++)
    {
        news_rounds[round][random].fluctuations[i] = -news_rounds[round][random].fluctuations[i];
    }
}

void checkCharacter3()
{
    for (int i = 0; i < 2; i++)
    {
        if (players[i].character == 3)
        {
            if (players[i].isAchieved == -1)
                continue;
            if (prevPoints[i] * 0.9 <= players[i].points)
                players[i].isAchieved = -1;
        }
    }
}

void claimWinner()
{
    char bufferWin[MAXLINE], bufferLose[MAXLINE], bufferTie[MAXLINE];
    if (players[0].points > players[1].points)
    {
        sprintf(bufferWin, "Congratulations, %s! You won the game!\n", players[0].name);
        Writen(players[0].connfd, bufferWin, strlen(bufferWin));
        sprintf(bufferLose, "So sorry, %s. You lost the game!\n", players[1].name);
        Writen(players[1].connfd, bufferLose, strlen(bufferLose));
    }
    else if (players[1].points > players[0].points)
    {
        sprintf(bufferWin, "Congratulations, %s! You won the game!\n", players[1].name);
        Writen(players[1].connfd, bufferWin, strlen(bufferWin));
        sprintf(bufferLose, "So sorry, %s. You lost the game!\n", players[0].name);
        Writen(players[0].connfd, bufferLose, strlen(bufferLose));
    }
    else
    {
        sprintf(bufferTie, "It's a tie!\n");
        Writen(players[0].connfd, bufferTie, strlen(bufferTie));
        Writen(players[1].connfd, bufferTie, strlen(bufferTie));
    }
    memset(bufferWin, 0, sizeof(bufferWin));
    memset(bufferLose, 0, sizeof(bufferLose));
    memset(bufferTie, 0, sizeof(bufferTie));
}