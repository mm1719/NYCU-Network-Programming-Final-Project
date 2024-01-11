#include "unp.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "ansi_escape.h"

char id[MAXLINE];

const char instruction_format[500] =
    "指令:\n"
    "做多:\x1b[31mlong 商品號碼 數量(錢$/件數)\n"
    "    \x1b[0mex: 買一件100元的5號商品\n"
    "      long 5 $100  (or)  long 5 1\n"
    "做空:\x1b[32mshort 商品號碼 數量(錢$/件數)\n"
    "    \x1b[0mex: 做空一件100元的5號商品\n"
    "      short 5 $100  (or)  short 5 1\n"
    "獲取更詳細的新聞內容:\x1b[36minfo\n"
    "\x1b[0m下一局釋放假消息迷惑對手:\x1b[36mfake\n"
    "\x1b[0m向銀行貸款:\x1b[36mloan 貸款金額\n"
    "\t\x1b[0mex: 貸款1000元\n"
    "\t\tloan 1000\n"
    "\x1b[0m結束這回合:\x1b[36mfinish\x1b[0m\n";

int isdigitstr(char *str, int ini)
{
    int len = strlen(str);
    int i;

    for (i = ini; i < len; i++)
    {
        if (str[i] != '\n')
            if (!(isdigit(str[i])))
                return 0;
    }
    return 1;
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

int split(char dst[][80], char *str, const char *spl)
{
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while (result != NULL)
    {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}

void xchg_data(FILE *fp, int sockfd)
{
    int maxfdp1, stdineof, peer_exit, n;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];

    Writen(sockfd, id, strlen(id)); // send id to server
    clearScreen();
    set_scr();
    moveTo(0, 0);
    char dst[20][80];
    ssize_t len = Read(sockfd, recvline, MAXLINE); // recv:Welcome! Choose your character:...
    recvline[len] = '\0';
    int s_size = split(dst, recvline, "\n");
    // printf("%s", recvline);
    int i = 0;
    for (i = 0; i < s_size; i++)
    {
        printf("%s", dst[i]);
        if (i != (s_size - 1))
            printf("\n");
        if (i == 0 || i == 12)
            printf("\n");
    }

    int cha_no;
    while (1)
    {
        Fgets(sendline, MAXLINE, fp); // input charactor no.
        cha_no = sendline[0] - '0';
        n = strlen(sendline);
        if (cha_no >= 0 && cha_no <= 3 && n == 2)
        {
            break;
        }
        moveUp(1);
        clearLine();
        setTextColor(RED_TXT);
        printf("%s", dst[s_size - 1]);
    }
    resetColor();
    sendline[n] = '\n';
    Writen(sockfd, sendline, n + 1);

    // Round 1 begin
    memset(recvline, 0, sizeof(recvline));
    clearScreen();
    moveTo(0, 0);

    len = Read(sockfd, recvline, MAXLINE); // recv: Round 1 message
    recvline[len] = '\0';
    char *token = strtok(recvline, "\n");
    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, "\n");
    }
    //printf("%s", recvline);
    printf("\n");

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    maxfdp1 = sockfd + 1;
    Select(maxfdp1, &rset, NULL, NULL, NULL);
    if (FD_ISSET(sockfd, &rset))
    { /* socket is readable */
        if (Readline(sockfd, recvline, MAXLINE) == 0)
            err_quit("str_cli: server terminated prematurely");
        Fputs(recvline, stdout);
    }

    /*len = Read(sockfd, recvline, MAXLINE); // recv: Round 1\n  You have 2or1,000,000 points!
    int player_points = 0, round;
    char tmp1[20];
    if (cha_no == 1)
    {
        player_points += 2000000;
    }
    else
    {
        player_points += 1000000;
    }
    sscanf(recvline, "%s %d", tmp1, &round);
    printf("%s", recvline);
    printf("\n\n");

    memset(recvline, 0, sizeof(recvline));*/

    /*char dst_2[20][80];
    int s_size2;
    len = Read(sockfd, recvline, MAXLINE); // recv: news
    s_size2 = (dst_2, recvline, "\n");

    for (i = 0; i < s_size2; i++)
    {
        printf("%s\n", dst[i]);
    }
    printf("\n");
    memset(recvline, 0, sizeof(recvline));*/

    /*len = Read(sockfd, recvline, MAXLINE); // recv: price
    recvline[len] = '\0';
    printf("%s", recvline);
    printf("\n\n");
    */
    //printf("不見了\n");
    printf("%s", instruction_format); // print instruction format
    printf("\n");
    printf("input:");
    memset(sendline, 0, sizeof(sendline));
    char sendline_2[MAXLINE];
    int instr;
    int bk = 0;
    while (!(bk == 1 && instr == 8))
    {
        int bk = 0;
        memset(sendline, 0, sizeof(sendline));
        Fgets(sendline, MAXLINE, fp); // input instruction.

        strcpy(sendline_2, sendline);
        instr = extract_instr(sendline_2);
        char dst_3[20][80];
        int s_size3;
        if (instr != -1)
        {

            char not_enough[MAXLINE];
            switch (instr)
            {
            case 1: // long with $dollar
                s_size3 = split(dst_3, sendline_2, " ");
                if (s_size3 == 3 && isdigitstr(dst_3[1], 0) && isdigitstr(dst_3[2], 1))
                {
                    bk = 1;
                }
                break;

            case 2: // long with amount
                s_size3 = split(dst_3, sendline_2, " ");
                if (s_size3 == 3 && isdigitstr(dst_3[1], 0) && isdigitstr(dst_3[2], 0))
                {
                    bk = 1;
                }
                break;

            case 3: // short with $dollar
                s_size3 = split(dst_3, sendline_2, " ");
                if (s_size3 == 3 && isdigitstr(dst_3[1], 0) && isdigitstr(dst_3[2], 1))
                {
                    bk = 1;
                }
                break;

            case 4: // short with amount
                s_size3 = split(dst_3, sendline_2, " ");
                if (s_size3 == 3 && isdigitstr(dst_3[1], 0) && isdigitstr(dst_3[2], 0))
                {
                    bk = 1;
                }
                break;

            case 5: // info
                bk = 1;
                break;

            case 6: // fake
                bk = 1;
                break;

            case 7: // loan
                s_size3 = split(dst_3, sendline_2, " ");
                if (s_size3 == 2 && isdigitstr(dst_3[1], 0) && isdigitstr(dst_3[2], 0))
                {
                    bk = 1;
                }
                break;

            case 8: // finish
                bk = 1;
                break;

                // case -1: //invalid
                //     break;
            }
        }
        if (bk)
        {
            moveUp(2);
            clearLine();

            Writen(sockfd, sendline, strlen(sendline));
            if (instr != 8)
            {
                moveUp(1);
                clearLine();

                memset(recvline, 0, sizeof(recvline));
                len = Read(sockfd, recvline, MAXLINE); // recv: feedback from server
                recvline[len] = '\0';
                printf("feedback:%s", recvline);
            }

            printf("\n");
            if (instr != 8)
            {
                clearLine();
                printf("input:");
            }
        }
        else
        {
            moveUp(3);
            clearLine();
            moveDown(1);
            clearLine();
            printf("\x1b[%dmwrong instruction format, please input again\n", RED_TXT);
            resetColor();
            clearLine();

            printf("input:");
        }
    }
    clearScreen();
    moveTo(0, 0);

    stdineof = 0;
    peer_exit = 0;
    for (;;)
    {
        FD_ZERO(&rset);
        maxfdp1 = 0;
        if (stdineof == 0)
        {
            FD_SET(fileno(fp), &rset);
            maxfdp1 = fileno(fp);
        };
        if (peer_exit == 0)
        {
            FD_SET(sockfd, &rset);
            if (sockfd > maxfdp1)
                maxfdp1 = sockfd;
        };
        maxfdp1++;
        Select(maxfdp1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset))
        { /* socket is readable */
            n = Read(sockfd, recvline, MAXLINE);
            if (n == 0)
            {
                if (stdineof == 1)
                    return; /* normal termination */
                else
                {
                    printf("(End of input from the peer!)");
                    peer_exit = 1;
                    return;
                };
            }
            else if (n > 0)
            {
                recvline[n] = '\0';
                printf("%s", recvline);
            }
            else
            { // n < 0
                printf("(server down)");
                return;
            };
        }

        if (FD_ISSET(fileno(fp), &rset))
        { /* input is readable */

            if (Fgets(sendline, MAXLINE, fp) == NULL)
            {
                if (peer_exit)
                    return;
                else
                {
                    printf("(leaving...)\n");
                    stdineof = 1;
                    Shutdown(sockfd, SHUT_WR); /* send FIN */
                };
            }
            else
            {
                n = strlen(sendline);
                sendline[n] = '\n';
                Writen(sockfd, sendline, n + 1);
            };
        }
    }
};

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 3)
        err_quit("usage: tcpcli <IPaddress> <ID>");

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT + 5);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    strcpy(id, argv[2]);

    Connect(sockfd, (SA *)&servaddr, sizeof(servaddr));

    xchg_data(stdin, sockfd); /* do it all */

    exit(0);
}
