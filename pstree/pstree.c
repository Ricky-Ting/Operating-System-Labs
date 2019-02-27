#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#define MAXPID 1<<21

struct node {
	pid_t pid_num;
	pid_t pid_p;
	int p_index;
	char pid_name[100] ;
	int child_num;
	int child_index[200];
} pidtree[MAXPID];

int counter = 0;
int root;
int p_mode=0;
int n_mode=0;
void pstree(void);
void print_tree(int no);
void printbefore(int index, int mode); 
void printspace(int len);
int main(int argc, char *argv[]) {
  int o;
	while( (o = getopt(argc,argv,"-pnV")) !=-1 ) {
	switch(o) {
		case 'p' : p_mode=1; break;
		case 'n' : n_mode=1; break;
		case 'V' : printf("pstree:V3.1415\n"); return 0; break;
		default : printf("wrong parameter\n"); return 0; break;
	}


	}


	pstree();

  return 0;
}

int is_number(char* s) {
	while(*s) {
		if( (*s < '0' ) || (*s > '9') )
			return 0;
		s++;
	}
	return 1;

}

void pstree() {
	DIR *dir = opendir("/proc");
	struct dirent * ptr;
	while( (ptr=readdir(dir)) ) {
		if( (ptr->d_type==DT_DIR) && (is_number(ptr->d_name)) ) {
			//printf("%s\n",ptr->d_name);
			//DIR *pid_dir = opendir(  strcat("/proc",ptr->d_name) );
			if( strcmp(ptr->d_name,"2")==0)
				continue;
			char tmp[10] = "/proc";
			char pid_dir[300], pid_name_f[300], pid_pa_f[300];
			sprintf(pid_dir,"%s/%s",tmp,ptr->d_name);
			sprintf(pid_name_f, "%s/comm",pid_dir);
			sprintf(pid_pa_f, "%s/stat",pid_dir);
			
			//printf("%s\n",pid_name_f);
			FILE * F_name = fopen(pid_name_f,"r");
			assert(F_name);
			fscanf(F_name,"%s",pidtree[counter].pid_name);	
			fclose(F_name);
	
			FILE * F_pa = fopen(pid_pa_f, "r");
			assert(F_pa);
			char omit1[100];
			char omit2;
			fscanf(F_pa,"%d %s %c %d",&pidtree[counter].pid_num, omit1, &omit2 ,&pidtree[counter].pid_p);	
			fclose(F_pa);
			int p = counter;
			counter++;		
			
	
			char pid_task_dir[300];
			sprintf(pid_task_dir,"%s/task",pid_dir);
			DIR * pid_task = opendir(pid_task_dir);
			struct dirent * subptr;
			while( (subptr=readdir(pid_task))  )	{
				if( (strcmp(subptr->d_name,ptr->d_name)!=0) && (is_number(subptr->d_name))  ) {
						char task_ppid_F[600];
						sprintf(task_ppid_F,"/proc/%s/task/%s/stat",ptr->d_name,subptr->d_name);	
						
					FILE * TASK_PPID = fopen(task_ppid_F,"r");
					assert(TASK_PPID);
					fscanf(TASK_PPID,"%d %s %c %d",&pidtree[counter].pid_num, omit1, &omit2 ,&pidtree[counter].pid_p);	
					pidtree[counter].pid_p= pidtree[p].pid_num;
					sprintf(pidtree[counter].pid_name,"{%s}",pidtree[p].pid_name);
					fclose(TASK_PPID);
					counter++;
				}	


			}
			closedir(pid_task);
			
			//printf("%s\n",strcat(tmp,ptr->d_name));
			//printf("%s,%s\n",tmp,pid_dir);
	

		}
	}
	closedir(dir);

	/*		
	for(int i=0;i<counter;i++) {
		printf("pid:%d,name:%s,parent:%d\n",pidtree[i].pid_num,pidtree[i].pid_name, pidtree[i].pid_p);
	}
	*/

	
	for(int i=0;i<counter;i++) {
		pidtree[i].child_num=0;
	}	

	for(int i=0;i<counter;i++) {	
		if(pidtree[i].pid_num==1) {
			root = i;
		}
		else {
			for(int j=0;j<counter;j++){
				if(pidtree[j].pid_num==pidtree[i].pid_p) {
					pidtree[j].child_index[pidtree[j].child_num]=i;
					pidtree[j].child_num++;
					pidtree[i].p_index=j;
				}
			}

		}
	}
	
	if(n_mode) {
		for(int i=0;i<counter;i++) {
			for(int j=0;j<pidtree[i].child_num;j++) {
				for(int k=pidtree[i].child_num-1; k>j; k-- ) {
					if(pidtree[pidtree[i].child_index[k]].pid_num <   pidtree[pidtree[i].child_index[k-1]].pid_num ) {
						int swaptmp = pidtree[i].child_index[k];
						pidtree[i].child_index[k]=pidtree[i].child_index[k-1];
						pidtree[i].child_index[k-1]=swaptmp;
					}
				}
			}	

		}	

	}
	print_tree(root);

}

void print_tree(int no){
	printbefore(no,1);
	printf("%s",pidtree[no].pid_name);
	if(p_mode) {
		printf("(%d)",pidtree[no].pid_num);
	}
	for(int i=0;i<pidtree[no].child_num;i++) {
			print_tree(pidtree[no].child_index[i]);	
	}		
	printf("\n");
}

void printbefore(int index, int mode) {
	if(index==root)	
		return;
			
	if( pidtree[pidtree[index].p_index].child_index[0] ==index && mode) {
		printf("-+-");
		return;
	}
	else {
		printbefore(pidtree[index].p_index,0);
		if(pidtree[index].p_index==root)
			printspace(strlen(pidtree[pidtree[index].p_index].pid_name)+1);
		else
			printspace(strlen(pidtree[pidtree[index].p_index].pid_name)+2);
	}
	printf("|");
	if(mode){
		printf("-");
	}
}


void printspace(int len) {
	for(int i=0;i<len;i++) {
		printf(" ");
	}

}
