
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "keyboard.h"

/*======================================================================*
                            kernel_main
 *======================================================================*/
int UserFlag=0;
char user[2][64];
char password[2][64];
char WinMatches[2][16];
char preuser[128];
char prepassword[128];
char prematch[32];
char userAfile[16][256];
char userBfile[16][256];
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	struct task* p_task;
	struct proc* p_proc= proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16   selector_ldt = SELECTOR_LDT_FIRST;
        u8    privilege;
        u8    rpl;
	int   eflags;
	int   i, j;
	int   prio;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
	        if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio      = 15;
                }
                else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
			prio      = 5;
                }

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid = i;			/* pid */

		p_proc->run_count = 0;

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		/* p_proc->nr_tty		= 0; */

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_proc->ticks = p_proc->priority = prio;
		p_proc->run_state = 1;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}
	proc_table[5].run_state = 0;
	proc_table[6].run_state = 0;
	proc_table[7].run_state = 0;
        /* proc_table[NR_TASKS + 0].nr_tty = 0; */
        /* proc_table[NR_TASKS + 1].nr_tty = 1; */
        /* proc_table[NR_TASKS + 2].nr_tty = 1; */

	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}


/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}


/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	int fd;
	int i, n;
	char tty_name[] = "/dev_tty0";

	char rdbuf[128];


	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

//	char filename[MAX_FILENAME_LEN+1] = "zsp01";
	const char bufw[80] = {0};
//	const int rd_bytes = 3;
//	char bufr[rd_bytes];
	InitUser();
	clear();
	  for(int i=0;i<10;i++)
    {
        for(int j=0;j<i;j++)
        {
            printf("\n");
        }
	    printf("   C     C  CCCCCC  C      C          CC               CC          CCCCCC      \n");
	    printf("   C     C  C       C      C         C  C             C  C         C           \n"); 
	    printf("   C     C  C       C      C        C    C           C    C        C           \n");
	    printf("   C     C  C       C      C       C      C         C      C       C           \n");
	    printf("   C     C  C       C      C      C        C       C        C      C           \n");
	    printf("   CCCCCCC  CCCCCC  C      C     C          C     C          C     CCCCCC      \n");
	    printf("   C     C  C       C      C      C        C       C        C           C      \n");
	    printf("   C     C  C       C      C       C      C         C      C            C      \n");
	    printf("   C     C  C       C      C        C    C           C    C             C      \n");
	    printf("   C     C  C       C      C         C  C             C  C              C      \n");
	    printf("   C     C  CCCCCC  CCCCCC CCCCCC     CC               CC        CCCCCCCC      \n");
	    sleep(1);
	    clear();
    }    
			printf("        *****************************************************\n");
 			printf("        *                                                   *\n");
			printf("        *        welcome to my      My tiny OS!             *\n");
			printf("        *      		                                        *\n");
			printf("        *          by 1552736 shikaijie                     *\n");
            printf("        *          by 1552662 fangweihao                    *\n");
			printf("        *          by 1556121 liziwang                      *\n");
			printf("        *                                                   *\n");
			printf("        *****************************************************\n");
	while (1) {
		if(UserFlag==0)
		{
			printl("[root@guest /] ");
		}
		else
		{
			printl("[root@user /]");
		}
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
	 	//show();
				if (strcmp(rdbuf, "help") == 0)
				{
					help();
				}
                else if (strcmp(rdbuf, "talk") == 0)
				{		
                    clear();
					printf("hello!!!this is your system,what can I do for you?\n");
					while (1) 	
					{
						printl("[You /] ");
						int r = read(fd_stdin, rdbuf, 70);
						rdbuf[r] = 0;
                		if (strcmp(rdbuf, "Who are you") == 0)
                		{
							printf("The tiny Operating System");
							continue;
                		}
						else if (strcmp(rdbuf, "What's the most wise saying?") == 0)
						{
							printf("Never Give Up Your Passion Towards Life\n");
							continue;
						}
                		else if (strcmp(rdbuf, "How old are you") == 0)
						{
							printf("I live as long as The Seven Kingdom \n");
							continue;
						}
						else if (strcmp(rdbuf, "how are you?") == 0)
						{
							printf(" Well,that'a good question.Miserable,but alive\n");
							continue;
						}	
						else if (strcmp(rdbuf, "Exit") == 0)
						{
							clear();
							printf("goodbye,welcome to using me Again!\n");
							milli_delay(1500);
       					 	break;
						}
						else
							printf("sorry,I don't understand what you said...\n");
					}
				}	
				else if (strcmp(rdbuf, "runttt") == 0)
				{
					TTT(fd_stdin, fd_stdout);
				}
				else if (strcmp(rdbuf, "clear") == 0)
				{
					clear();
				}
				else if(strcmp(rdbuf,"wuziqi")==0)
				{
					GoBang(fd_stdin);
				}
				else if(strcmp(rdbuf,"calculator")==0)
				{
					mycalculator(fd_stdin);
				}
				else if(strcmp(rdbuf,"2048")==0)
				{
					init2048(fd_stdin);
				}
				else if(strcmp(rdbuf,"testdocument")==0)
				{
					InitUser(fd_stdin);
				}
				else if(strcmp(rdbuf,"login")==0)
				{
					login(fd_stdin);
				}
				else if(strcmp(rdbuf,"logout")==0)
				{
					logout();
				}
				else if(strcmp(rdbuf,"change")==0)
				{
					ChangeInfo(fd_stdin);
				}
				else if(strcmp(rdbuf,"init")==0)
				{
					initialize();
				}
				else if(strcmp(rdbuf,"pause a") == 0) {
					proc_table[4].run_state = 0 ;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"pause b") == 0 ){
					proc_table[5].run_state = 0 ;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"pause d") == 0 ){
					proc_table[7].run_state = 0 ;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"restart b") == 0 ){
				}
				else if(strcmp(rdbuf,"restart c") == 0 ){
					
				}
		        else if(strcmp(rdbuf,"restart d") == 0)
		        {

		        }
				else if(strcmp(rdbuf,"kill b") == 0 ){
					proc_table[5].p_flags = 1;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"kill c") == 0 ){
					proc_table[6].p_flags = 1;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"kill d") == 0 ){
					proc_table[7].p_flags = 1;
					ProcessManage();
				}

				else if(strcmp(rdbuf,"pause c") == 0 ){
					proc_table[6].run_state = 0 ;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"resume a") == 0 ){
					proc_table[4].run_state = 1 ;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"resume d") == 0 ){
					proc_table[7].run_state = 1 ;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"up a") == 0 ){
					proc_table[4].priority = proc_table[4].priority*2;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"up c") == 0 ){
					proc_table[6].priority = proc_table[6].priority*2;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"up b") == 0 ){
					proc_table[5].priority = proc_table[5].priority*2;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"up d") == 0 ){
					proc_table[7].priority = proc_table[7].priority*2;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"resume b") == 0 ){
					proc_table[5].run_state = 1 ;
					ProcessManage();
				}
				else if(strcmp(rdbuf,"resume c") == 0 ){
				proc_table[6].run_state = 1 ;
				ProcessManage();
				}	
				else
				{
					printf("Command not found, please check!\n");
				}	
}
}
int BOARD_SIZE=30;//initialize array's length,throught the exception
char board[30][30];//chessboard
int cnt;//the number of written point
char qizi[2]={'F','S'};
//search in 8 directions to ensure whether it's connected
int dx[8]={-1,0,-1,-1,1,1,0,1};
int dy[8]={0,-1,-1,1,-1,1,1,0};
void InitChessBoard()
{
	cnt=0;
	for(int i=1;i<15;i++)
	{
		for(int j=1;j<=15;j++)
		{
			board[i][j]='-';
		}
	}
}
void PrintChess()
{
	int i,j;
	for(i=1;i<=15;i++)
	{
		for(j=1;j<=15;j++)
		{
			printf("%c ",board[i][j]);
		}
		printf("\n");
	}
}
int CheckInput(int x,int y)
{
	if(board[x][y]!='-')
	{
		return 0;
	}
	if(x<1||x>15||y<1||y>15)
	{
		return 0;
	}
	return 1;
}
int Check(int x,int y)
{
	if(x>15||x<1||y>15||y<1)
		return 0;
	return 1;
}
int CheckWinner(int num,int x,int y)
{
	for(int i=0;i<4;i++)
	{
		int sum=1;
		int xx=x+dx[i];
		int yy=y+dy[i];
		while(board[xx][yy]==qizi[num]&&Check(xx,yy)==1)
		{
			sum++;
			xx=xx+dx[i];
			yy=yy+dy[i];
		}
		xx=x+dx[7-i];
		yy=y+dy[7-i];
		while(board[xx][yy]==qizi[num]&&Check(xx,yy)==1)
		{
			sum++;
			xx=xx+dx[7-i];
			yy=yy+dy[7-i];
		}
		if(sum>=5)
		{
			return 1;
		}
	}
	return 0;
}
int run(int fd_stdin)
{
	InitChessBoard();
	char bufr[128];
	int column,row;
	//the default player,zero
	int num=0;
	while(1)
	{
		PrintChess();
		memset(bufr,0,128);
		printf("input the column,please;\n");
		read(fd_stdin,bufr,128);
		column=getNum(bufr);
		memset(bufr,0,128);
		printf("input the row,please;\n");
		read(fd_stdin,bufr,128);
		row=getNum(bufr);
		if(CheckInput(column,row)==0)
		{
			printf("the input has invalid characters,please input again!\n");
			return -1;
		}
		board[column][row]=qizi[num];
		cnt++;
		if(CheckWinner(num,column,row)==1)
		{
			return num+1;
		}
		num=1-num;
	}
}
void GoBang(int fd_stdin)
{
	if(UserFlag)
	{
		printf("The GoBang Game Started!\n");
		printf("In the history,player1 has won %s matches,and player2 has won %s matches\n",WinMatches[0],WinMatches[1]);
		int number=run(fd_stdin);
		if(number!=-1)
		{
			PrintChess();
			printf("player %d has won the game",number);
			//WinMatches[number-1]=WinMatches[number-1]+1;
			//printf("player1 has won %s matches,and player2 has won %s matches\n",WinMatches[0],WinMatches[1]);
		}
	}
	else
	{
		printf("sorry,you are not in user's mode,you can't play the game!\n");
	}
}
/*calculator*/
int getNum(char* buffer){
	int num=0;
	for (int i = 0; i < strlen(buffer); i++)
	{
		num=num*10+(buffer[i]-'0');
	}
	return num;
}
void mycalculator(int fd_stdin)
{
	int i, num1 = 0, num2 = 0, flag = 1, res = 0;
	char bufr[128];
	printf("***************************************************\n");
	printf("*           tiny    Calculator(press 'e' to quit) *\n");
	printf("***************************************************\n");
	printf("***************************************************\n");
	printf("***************************************************\n\n");
	while(flag == 1){	
		memset(bufr,0,128);
		printf("Please input num1:");
		i = read(fd_stdin, bufr, 128);
		if (bufr[0] == 'e')
			break;
		num1 = getNum(bufr);
		printf("num1: %d\n", num1);
		memset(bufr,0,128);
		printf("Please input num2:");
		i = read(fd_stdin, bufr, 128);	
		if (bufr[0] == 'e')
			break;
		num2 = getNum(bufr);
		printf("num2: %d\n", num2);
		memset(bufr,0,128);
		printf("Please input operator:");
				i = read(fd_stdin, bufr, 1);
		switch(bufr[0])
		{
			case '+':
				res = num1 + num2;
				printf("%d + %d = %d\n", num1, num2, res);
				break;
			case '-':
				res = num1 - num2;
				printf("%d - %d = %d\n", num1, num2, res);
				break;
			case '*':
				res = num1 * num2;
				printf("%d * %d = %d\n", num1, num2, res);
				break;
			case '/':
				if(num2 <= 0)
				{
					printf("warning:num2 can't be zero!\n");
					break;
				}
				res = num1 / num2;
				printf("%d / %d = %d\n", num1, num2, res);
			case 'e':
				flag = 0;
				break;
			default:
				printf("No such command!\n");
		}
	}
	return 0;
}
/*caculator*/
/*2048*/
unsigned int _seed2 = 0xDEADBEEF;

void srand(unsigned int seed){
    _seed2 = seed;
}

int rand() {
    unsigned int next = _seed2;
    unsigned int result;

    next *= 1103515245;
    next += 12345;
    result = ( unsigned int  ) ( next / 65536 ) % 2048;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= ( unsigned int ) ( next / 65536 ) % 1024;

    next *= 1103515245;
    next += 12345;
    result <<= 10;
    result ^= ( unsigned int ) ( next / 65536 ) % 1024;

    _seed2 = next;

    return result;
}

int mat_init(int *mat);

int mat_left(int *mat);
int mat_right(int *mat);
int mat_up(int *mat);
int mat_down(int *mat);

int mat_reach(int *mat);
int mat_insert(int *mat);
void mat_print(int *mat);

void init2048(int fd_stdin){
    int mat[16] = {0};
    int state = 0;
    char keys[128];
    printf("**************************************\n");
    printf("*               2048                 *\n");
    printf("**************************************\n");
    printf("*  1. Enter a to go left             *\n");
    printf("*  2. Enter w to go up               *\n");
    printf("*  3. Enter s to go down             *\n");
    printf("*  4. Enter d to go right            *\n");
    printf("*  5. Enter q to quit                *\n");
    printf("**************************************\n");
    while(1){
        printl("Init Matrix\n");
        mat_init(mat);

        while(1){
            printf("type in the direction(w a s d):");
            ClearArr(keys, 128);
            int r = read(fd_stdin, keys, 128);

            if (strcmp(keys, "a") == 0)
            {
                state = mat_left(mat);
            }
            else if (strcmp(keys, "s") == 0)
            {
                state=mat_down(mat);
            }
            else if (strcmp(keys, "w") == 0)
            {
                state=mat_up(mat);
            }
            else if (strcmp(keys, "d") == 0)
            {
                state=mat_right(mat);
            }
            else if (strcmp(keys, "q") == 0)
            {
                return 0;
            }
            else
            {
                printl("Input Invalid, Please retry\n");
                continue;
            }
            if(state==0){
                printl("can't add,try again!\n");
                continue;
            }
            if(mat_reach(mat)){
                printf("You Win\n");
                break;
            }
            if(!mat_insert(mat)){
                printf("You Lose\n");
                break;
            }
            mat_print(mat);
        }
        printf("another one?(y or n):");
        ClearArr(keys, 128);
        int r = read(0, keys, 128);
        if (strcmp(keys, "n")==0)
        {
            break;
        }
    }
    return 0;
}

int mat_init(int *mat)
{
    int i, j;
    //给一个随机数
    /*srand(546852);*/
    mat_insert(mat);
    mat_insert(mat);
    mat_print(mat);
    return 0;
}

int mat_left(int *mat){
    printl("Left\n");

    int i,j;
    int flag=0;
    int k=0,temp[4]={0},last=0;
    for(i=0;i<4;i++){
        memset(temp,0,sizeof(int)*4);
        for(j=0,k=0,last=0;j<4;j++){
            if(mat[i*4+j]!=0){
                temp[k]=mat[i*4+j];
                mat[i*4+j]=0;
                last=j+1;
                k++;
            }
        }
        if(k<last) flag=1;
        for(j=0;j<3;j++){
            if(temp[j]>0&&temp[j]==temp[j+1]){
                temp[j]+=temp[j];
                temp[j+1]=0;
                flag=1;
            }
        }
        for(j=0,k=0;k<4;k++){
            if(temp[k]!=0){
                mat[i*4+j]=temp[k];
                j++;
            }
        }
    }
    return flag;
}

int mat_right(int *mat){
    printl("Right\n");

    int i,j;
    int flag=0;
    int k=0,temp[4]={0},last=0;
    for(i=0;i<4;i++){
        memset(temp,0,sizeof(int)*4);
        for(j=3,k=3,last=3;j>=0;j--){
            if(mat[i*4+j]!=0){
                temp[k]=mat[i*4+j];
                mat[i*4+j]=0;
                last=j-1;
                k--;
            }
        }
        if(k>last) flag=1;
        for(j=3;j>=0;j--){
            if(temp[j]>0&&temp[j]==temp[j+1]){
                temp[j]+=temp[j];
                temp[j+1]=0;
                flag=1;
            }
        }
        for(j=3,k=3;k>=0;k--){
            if(temp[k]!=0){
                mat[i*4+j]=temp[k];
                j--;
            }
        }
    }
    return flag;
}

int mat_up(int *mat){
    printl("Up\n");

    int i,j;
    int flag=0;

    int k=0,temp[4]={0},last=0;
    for(i=0;i<4;i++){
        memset(temp,0,sizeof(int)*4);
        for(j=0,k=0,last=0;j<4;j++){
            if(mat[j*4+i]!=0){
                temp[k]=mat[j*4+i];
                mat[j*4+i]=0;
                last=j+1;
                k++;
            }
        }
        if(k<last) flag=1;
        for(j=0;j<3;j++){
            if(temp[j]>0&&temp[j]==temp[j+1]){
                temp[j]+=temp[j];
                temp[j+1]=0;
                flag=1;
            }
        }
        for(j=0,k=0;k<4;k++){
            if(temp[k]!=0){
                mat[j*4+i]=temp[k];
                j++;
            }
        }
    }
    return flag;
}

int mat_down(int *mat){
    printl("Down\n");

    int i,j;
    int flag=0;
    int k=0,temp[4]={0},last=0;
    for(j=0;j<4;j++){
        memset(temp,0,sizeof(int)*4);
        for(i=3,k=3,last=3;i>=0;i--){
            if(mat[i*4+j]!=0){
                temp[k]=mat[i*4+j];
                mat[i*4+j]=0;
                last=i-1;
                k--;
            }
        }
        if(k>last) flag=1;
        for(i=3;i>0;i--){
            if(temp[i]>0&&temp[i]==temp[i-1]){
                temp[i]+=temp[i];
                temp[i-1]=0;
                flag=1;
            }
        }
        for(i=3,k=3;k>=0;k--){
            if(temp[k]!=0){
                mat[i*4+j]=temp[k];
                i--;
            }
        }
    }
    return flag;
}

int mat_reach(int *mat){
    int i, j;
    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            if(mat[i*4+j] == 2048)
                return 1;
        }
    }
    return 0;
}

int mat_insert(int *mat){
    char temp[16] = {0};
    int i, j, k = 0;
    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            if(mat[i*4+j] == 0){
                temp[k] = 4 * i + j;
                k++;
            }
        }
    }
    if(k == 0) return 0;
    k = temp[rand() % k];
    //随便给一个地方2或者4
    mat[((k-k%4)/4)*4+k%4]=2<<(rand()%2);
    return 1;
}

void mat_print(int *mat){
    int i, j;
    for(i = 0; i < 4; i++){
        for(j = 0; j < 4; j++){
            //这里需要规格化
            printf("%4d", mat[i*4+j]);
        }
        printf("\n");
    }
}

void ClearArr(char *arr, int length)
{
    int i;
    for (i = 0; i < length; i++)
        arr[i] = 0;
}

/*2048*/
/*my animation*/

/*my animation*/
/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	while(1)
	{
		if(proc_table[5].run_state == 1)
		{
			disp_str("b");
			milli_delay(900);
		}
	}  
}


void DocumentCover(char* filepath)
{
	int fd = -1;
	int n, i = 0;
	char bufr[128];
	char last[128];
	memset(bufr,0,128);


	fd=open(filepath,O_RDWR);
	n=write(fd,bufr,128);
	assert(n==128);
	close(fd);


	fd=open(filepath,O_RDWR);
	n=read(fd,last,128);
	printf("the current file:%s\n",last);
	close(fd);
}

void login(int fd_stdin)
{
	printf("user[0]:%s\n",user[0]);
	printf("user[1];%s\n",user[1]);
	char namebuf[64];
	char passbuf[64];
	memset(namebuf,0,64);
	memset(passbuf,0,64);
	printf("please input user name\n");
	read(fd_stdin,namebuf,64);
	printf("input name:%s\n",namebuf);
	if(strcmp(namebuf,user[0])==0||strcmp(namebuf,user[1])==0)
	{
		printf("please input password\n");
		read(fd_stdin,passbuf,64);
		if(strcmp(passbuf,password[0])==0||strcmp(passbuf,password[1])==0)
		{
			printf("login success!\n");
			UserFlag=1;
		}
		else
		{
			printf("password wrong,please retry\n");
		}
	}
	else
	{
		printf("no such users,please try again!\n");
	}
}
void logout()
{
	if(UserFlag)
	{
		printf("logout success!\n");
		UserFlag=0;
	}
	else
	{
		printf("you are not in user mode,you can't logout!\n");
	}
}
void ReadinMatches()
{
	int fd=-1;
	int n;
	char buffer[32];
	memset(buffer,0,32);
	fd=open("matches",O_RDWR);
	assert(fd!=-1);
	for(int i=0;i<32;i++)
	{
		if(i<16)
		{
			if(WinMatches[0][i]=='\0')
				buffer[i]=' ';
			else
				buffer[i]=WinMatches[0][i];
		}
		else 
		{
			if(WinMatches[0][i]=='\0')
				buffer[i]=' ';
			else
				buffer[i]=WinMatches[1][i-16];
		}
	}
	buffer[31]='\0';
	printf("buffer:%s\n",buffer );
	n=write(fd,buffer,32);
	assert(n==32);
	close(fd);
}
void ReadinPassword()
{
	int fd=-1;
	int n;
	char buffer[128];
	memset(buffer,0,128);
	fd=open("passwords",O_RDWR);
	assert(fd!=-1);
	for(int i=0;i<128;i++)
	{
		if(i<64)
		{
			if(password[0][i]=='\0')
				buffer[i]=' ';
			else
				buffer[i]=password[0][i];
		}
		else 
		{
			if(password[0][i]=='\0')
				buffer[i]=' ';
			else
				buffer[i]=password[1][i-64];
		}
	}
	buffer[127]='\0';
	printf("buffer:%s\n",buffer );
	n=write(fd,buffer,128);
	assert(n==128);
	close(fd);
}
void ReadinUser()
{
	int fd=-1;
	int n;
	char buffer[128];
	memset(buffer,0,128);
	fd=open("users",O_RDWR);
	assert(fd!=-1);
	for(int i=0;i<128;i++)
	{
		if(i<64)
		{
			if(user[0][i]=='\0')
				buffer[i]=' ';
			else
				buffer[i]=user[0][i];
		}
		else 
		{
			if(user[0][i]=='\0')
				buffer[i]=' ';
			else
				buffer[i]=user[1][i-64];
		}
	}
	buffer[127]='\0';
	printf("buffer:%s\n",buffer );
	n=write(fd,buffer,128);
	assert(n==128);
	close(fd);
}

void initialize()
{
	int fd=-1;
	int n;
	const char buffer1[128]="shikaijie   fangweihao";
	const char buffer2[128]="sskkjj123456 2253369";
	const char buffer3[32]="23 45";
	const char usera[256]="I love you baby @   I need you baby  @";
	const char userb[256]="It's such a shame@  I swear you don't want to see this  @";

	fd=open("users",O_RDWR);
	assert(fd!=-1);
	n=write(fd,buffer1,128);
	assert(n==128);
	close(fd);
	fd=open("passwords",O_RDWR);
	assert(fd!=-1);
	n=write(fd,buffer2,128);
	assert(n==128);
	close(fd);
	fd=open("matches",O_RDWR);
	assert(fd!=-1);
	n=write(fd,buffer3,32);
	assert(n==32);
	fd=open('userafile',O_CREAT|O_RDWR);
	assert(fd!=-1);
	n=write(fd,usera,256);
	assert(n==256);
	close(fd);
	fd=open('userbfile',O_CREAT|O_RDWR);
	assert(fd!=-1);
	n=write(fd,userb,256);
	assert(n==256);
	printf("initialization success\n");
	InitUser();
}

void ChangeInfo(int fd_stdin)
{
	if(UserFlag==0){
		printf("sorry,you are not in user mode,you can't change\n");
		return;
	}
	char diskbuffer[128];
	char buffer[64];
	char infobuf[64];
	memset(diskbuffer,0,128);
	memset(infobuf,0,64);
	memset(buffer,0,64);
	printf("please select the information you want to change:(name,password or detail)\n");
	read(fd_stdin,buffer,64);
	//printf("buffer:%s\n",buffer);
	if(strcmp(buffer,"name")==0)
	{
		printf("please input the old name \n");
		read(fd_stdin,infobuf,64);
		for(int i=0;i<2;i++)
		{
			if(strcmp(user[i],infobuf)==0)
			{
				memset(infobuf,0,64);
				printf("please input the new name\n");
				read(fd_stdin,infobuf,64);
				for(int j=0;j<64;j++)
				{
					user[i][j]=infobuf[j];
				}
				printf("operation succeed! new username:%s\n",user[i]);
				DocumentCover("users");
				ReadinUser();
				InitUser();
				return;
			}
		}
		printf("the user deoesn't exist,please check again!%s\n");
		return;
	}
	else if(strcmp(buffer,"password")==0)
	{
		printf("please input the user name\n");
		read(fd_stdin,infobuf,64);
		for(int i=0;i<2;i++)
		{
			if(strcmp(user[i],infobuf)==0)
			{
				memset(infobuf,0,64);
				printf("please input the new password\n");
				read(fd_stdin,infobuf,64);
				for(int j=0;j<64;j++)
				{
					password[i][j]=infobuf[j];
				}
				printf("operation succeed! new password:%s\n",password[i]);
				DocumentCover("passwords");
				ReadinPassword();
				InitUser();
				return;
			}
		}
		printf("the user doesn't exist,can't change password,please check again!\n" );
		return;
	}
	else if(strcmp(buffer,"detail")==0)
	{
		printf("please input the user name\n");
		read(fd_stdin,infobuf,64);
		for(int i=0;i<2;i++)
		{
			if(strcmp(user[i],infobuf)==0)
			{
				memset(infobuf,0,64);
				printf("the old detail is :%s\n",WinMatches[i]);
				printf("please input the new content\n");
				read(fd_stdin,infobuf,64);
				for(int j=0;j<16;j++)
				{
					WinMatches[i][j]=infobuf[j];
				}
				printf("operation succeed! new detail:%s\n",WinMatches[i]);
				DocumentCover("matches");
				ReadinMatches();
				InitUser();
				return;
			}
		}
		printf("the user doesn't exist,can't change detail,please check again!\n" );
		return;
	}
	else 
	{
		printf("sorry,wrong command!\n");
	}
}

void InitUser(int fd_stdin)
{
	int fd=-1;
	int n;
	//save the information to memory
	fd=open("users",O_RDWR);
	n=read(fd,preuser,128);
	close(fd);
	fd=open("passwords",O_RDWR);
	n=read(fd,prepassword,128);
	close(fd);
	fd=open("matches",O_RDWR);
	n=read(fd,prematch,32);
	assert(n==32);
	close(fd);
	//select the information to array
	int i=0,count=0,p=0;
	for(i=0;i<128;i++)
	{
		if(preuser[i]=='\0')
			break;
		else
		{
			if(preuser[i]==' ')
			{
				count++;
				p=0;
				while(preuser[i]==' ')
				{
					i++;
				}
				i--;
			}
			else
			{
				user[count][p]=preuser[i];
				p++;
			}
		}
	}
	count=0;p=0;
	for(i=0;i<128;i++)
	{
		if(prepassword[i]=='\0')
			break;
		else
		{
			if(prepassword[i]==' ')
			{
				count++;
				p=0;
				while(prepassword[i]==' ')
				{
					i++;
				}
				i--;
			}
			else
			{
				password[count][p]=prepassword[i];
				p++;
			}
		}
	}
	count=0;p=0;
	for(i=0;i<32;i++)
	{
		if(prematch[i]=='\0')
			break;
		else
		{
			if(prematch[i]==' ')
			{
				count++;
				p=0;
				while(prematch[i]==' ')
				{
					i++;
				}
				i--;
			}
			else
			{
				WinMatches[count][p]=prematch[i];
				p++;
			}
		}
	}
}

void TestC()
{	
	while(1)
	{
		if(proc_table[6].run_state == 1)
		{
			disp_str("c");
			milli_delay(900);
		}
	}       
}

void TestD()
{
    while(1)
    {
     	if(proc_table[7].run_state == 1){
	    disp_str("d");
	    milli_delay(450);
	}

    }
}


void ProcessManage()
{
	int i;
	clear();
	printf("=============================================================================\n");
	printf("============================Process Manager==================================\n");
	printf("=                                                                           =\n");
	printf("===========   myID      |    name       | spriority    | running?  =========\n");
	//进程号，进程名，优先级，是否是系统进程，是否在运行
	printf("=---------------------------------------------------------------------------=\n");
	for ( i = 0 ; i < NR_TASKS + NR_PROCS ; ++i )//逐个遍历
	{
//		if ( proc_table[i].priority == 0) continue;//系统资源跳过
		 
        if(proc_table[i].p_flags == 0){
		printf("======         %d              %s             %d           %d  ============\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority,proc_table[i].run_state) ;
        }
		else{
		printf("======         %d              %s             %d           %d  ============\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority,proc_table[i].run_state);
		}
    }
	printf("=============================================================================\n");
	printf("=          tips: use 'pause a/b/c' command -> you can pause one process    =  \n");
	printf("=          	use 'resume a/b/c' command ->you can resume one process    =\n");
	printf("=               use 'show all process' -> no hidden process                =\n");
	printf("=               use 'up a/b/c' -> higher process priority                  =\n");
	printf("=============================================================================\n");
}

void TTT(int fd_stdin,int fd_stdout)
{

}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}

void clear()
{
	clear_screen(0,console_table[current_console].cursor);
	console_table[current_console].crtc_start = 0;
	console_table[current_console].cursor = 0;
	
}

//void show()
//{
//	printf("%d  %d  %d  %d",console_table[current_console].con_size, console_table[current_console].crtc_start, console_table[current_console].cursor, console_table[current_console].orig);
//}

void help()
{
	printf("=============================================================================\n");
	printf("Command List     :\n");
	printf("1. process       : A process manage,show you all process-info here\n");
	printf("3. clear         : Clear the screen\n");
	printf("4. help          : Show this help message\n");
    printf("5. talk          : Do you want to talk to me?0.0\n");
	printf("6. 2048          : Run a 2048 game\n");
	printf("7.wuziqi         : Run a wuziqi game\n");
	printf("8.calculator     : Run a calculator game\n");
	printf("9.login          : user login the system\n");
	printf("10.logout        : user logout the system\n");
	printf("11.change        : user change information\n");
	printf("==============================================================================\n");		
}



void sleep(int pauseTime){
	int i = 0;
	for(i=0;i<pauseTime*1000000;i++)
	{
	}
}



