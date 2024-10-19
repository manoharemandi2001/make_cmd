#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct
{
    int size;
    long int target_time;
    char target[20];
    char (*depends)[20]; //dynamically allocated 2d array for depend files names
    long int *depends_time; //dynamically allocated 1d array for last modified timestamps of depend files
    char cmd[100];
}Make;

int over,pos,size_buff,makeflag;
int scnt=0; //total stuctures count

void getrule(int mfd,Make *info);
void getdepends(char *rule_buff, Make *info);
void get_cmd(int mfd, Make *info);
void* makedata(Make *info);
long int get_timestamp(char *filename);
void makecmd(Make *info);


int main()
{
    //system("clear");
    Make *info=NULL;
    info=makedata(info);
    makecmd(info);
}

void makecmd(Make *info)
{
    //start Check with 2nd rule
    for(int i=1; i<scnt; i++) //for structure
    {
        //compare target file time stamp with depend files time stamps
        for(int j=0; j<(info[i].size); j++) //for depends
        {
            if((info[i].depends_time[j])==-1)
            {
                printf("FAILURE NO DEPEND FILE..!");
            }
            else if((info[i].target_time)<(info[i].depends_time[j]))
            {
                makeflag=1;
                //print the command to be executed on console
                printf("%s\n",info[i].cmd);
                //execute the command
                system(info[i].cmd); //execute the command

                //refresh the database with new paramters of files
                info=NULL;
                //making all global variables to zero
                over=pos=size_buff=0;
                scnt=0; //total stuctures count
                info=makedata(info);
            }
        }
    }

    //Now Check 1st rule
    //compare target file time stamp with depend files time stamps
        for(int j=0; j<(info[0].size); j++) //for depends
        {
            if((info[0].target_time)<(info[0].depends_time[j]))
            {
                makeflag=1;
                //print the command to be executed on console
                printf("%s\n",info[0].cmd);
                //execute the command
                system(info[0].cmd); //execute the command
                return;
            }
        }

    if(makeflag==0)
    {
        printf("make: %s is up to date.\n",info[0].target);
    }
}




void* makedata(Make *info)
{
    
    Make temp;
    int mfd;
    
    //open makefile
    mfd= open("makefile", O_RDONLY);
    if(mfd<0)
    {
        perror("makefile not exist:");
        exit(0);
    }
   // printf("Makefile Opened Successfully\n"); //for debug
    over=lseek(mfd,0,SEEK_END); 
	//printf("end over=%d\n",over);
	lseek(mfd,0,SEEK_SET);//rewind the fd
          
        while(pos<over)
        {
            scnt++;
            info=realloc(info,(scnt)*sizeof(Make));

            //initialize temp structure
            //read rule line and extract target file and dependency files
            getrule(mfd,&temp);//for rule line

            //To read command line
            get_cmd(mfd,&temp); //for command line
            
            //copy temp structure to the dynamically allocated structure
            info[scnt-1]=temp;
            
            /*/////////////printfs for debug///////////////////////
            printf("\ntarget:%s\n",info[scnt-1].target); // printf("CHECK\n");
            printf("timestamp target_file:%ld\n",info[scnt-1].target_time);

            for(int j=0; j<(info[scnt-1].size); j++)
            {
              printf("depend%d:%s\n",j,(info[scnt-1].depends)[j]); //printf("CHECK\n");
              printf("timestamp depend_file%d:%ld\n",scnt-1, ((info[scnt-1]).depends_time)[j] );
            }
            printf("cmd line:%s\n",info[scnt-1].cmd); //printf("CHECK\n");
            //////////////////////////////////////////////////////*/

            pos=lseek(mfd,0,SEEK_CUR);
           // printf("cur_pos=%d\n",pos); //for debug
        }

    close(mfd);
return info;
}

void getrule(int mfd,Make *info)
{
    
    int i=0;
    char rule_buff[100];
    char target_buff[20];
    char ch;
    
    //read complete rule line //mfd moved till \n
    while(read(mfd,&ch,1)==1)
    {
        
        if(ch=='\n')
        {
            break;
        }

        rule_buff[i]=ch;
        i++;             
    }
    rule_buff[i]='\0'; 
    //rule line is ready

   //extract target file from rule line
    for(int i=0;(rule_buff[i])!=':'; i++)
    {
        target_buff[i] = rule_buff[i];
    }

    //To remove any spaces availabe in the file name
    strtok(target_buff," ");

    //Copy target file to the structure
    strcpy(info->target,target_buff);

    //Copy target file timestamp 
   // printf("target timestamp\n");  //for debug
    info->target_time= get_timestamp(target_buff);
    //printf("timestamp target_file:%ld\n",info->target_time); // for debug

    //Extract depend files from rule line
    getdepends(rule_buff, info);

}

void getdepends(char *rule_buff, Make *info)
{
    int i=0,cnt=0;
    char *temp;
    char (*depends)[20]=NULL;
    long int *depends_time=NULL;
    char ch;

    //To skip target file
    while(*rule_buff!=':')
    {
        rule_buff++;
    }rule_buff++;

    temp=rule_buff;
    
    while(temp=strtok(temp," "))
    {
        //printf("depend file:%s\n",temp); //for debug
        
        //allocate 2d array for depend files names
        cnt++;
        depends=realloc(depends, cnt*sizeof(*depends));
        //copy depend file names to 2darray
        strcpy(depends[cnt-1],temp);

        //allocate 1d array for depend files timestamps
        depends_time=realloc(depends_time, cnt*sizeof(*depends_time));

        //copy depend file timestamps to 1darray 
        //printf("depend timestamp\n"); //for debug
        depends_time[cnt-1]=get_timestamp(temp);// depends[cnt-1] (or) temp
        //printf("\ndepend time stamp(%s):%ld\n",temp,depends_time[cnt-1]); //for debug

        temp=NULL;
    }
    
    //copy 2d array depend files names to the structure(info->depends)
    info->depends =depends;    

    //copy 1d array depend files timestamps to the structure(info->depends_time)
    info->depends_time =depends_time;    

    //copy 2d array size to the structure
    info->size =cnt;
    //printf("size_cnt:%d\n",cnt);

//return depends;
}

void get_cmd(int mfd, Make *info)
{
    char cmd_buff[100];
    char ch;
    int i=0;

   //skip the tab
   //lseek(mfd,1,SEEK_CUR); 
   if(read(mfd,&ch,1)==1 && ch!='\t')
   {
        printf("Please provide <tab>\n");
        exit(0);
   }

    //read complete command line //mfd moved till \n
    while(read(mfd,&ch,1)==1)
    {
        if(ch=='\n')
        {
            break;
        }
        else if(ch== EOF)
        {
            over=1;
        }

        cmd_buff[i]=ch;
        i++;             
    }
    cmd_buff[i]='\0'; 

    //copy command line string to structure
    strcpy(info->cmd,cmd_buff);

    // Command line is ready
    //printf("cmd_buff:%s\n",cmd_buff); //for debugging
}

long int get_timestamp(char *filename)
{
    struct stat sb;

    if(stat(filename,&sb)==-1)
	 {
		 //perror("FAILURE in Time stamp!");
         return -1;
		 //exit(0);
	 }

    //To print date and time
    //printf("timestamp:%ld\n",sb.st_mtim.tv_sec);
    return (sb.st_mtim.tv_sec);
}
