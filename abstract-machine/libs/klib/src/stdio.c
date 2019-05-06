#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define true 1
#define false 0
#define bool char

void parse_str(int a, char str[]);
void parse_int(char *str ,int * a);
int printf(const char *fmt, ...) { /*maybe buggy */
  char buf[200];
	va_list ap;
	va_start(ap,fmt);
	int a=vsprintf(buf,fmt,ap);
	if(a<0)
			return a;
	//_putc('i');
	for(int i=0;buf[i]!='\0';i++) 
			_putc(buf[i]);	
	return a;

	
}

int vsprintf(char *out, const char *fmt, va_list ap) {
				*out='\0';
				while(*fmt!='\0') {
					char myflags='\0';
					char width[10]; width[0]='\0';
		/*			c har precision[10];
					char length='\0';
					char specifier='\0';
*/
					if(*fmt!='%') {
						*out=*fmt;
						out++;
						fmt++;
						*out='\0';
					}
					else {
			 					fmt++;
						// get_flags  
						while(*fmt==' ') {
							myflags=' ';
							fmt++;
						}	
						switch(*fmt) {
							case '-': myflags='-'; fmt++; break;
							case '+': myflags='+'; fmt++; break;
							case '#': myflags='#'; fmt++; break;
							case '0': myflags='0'; fmt++; break;
							default: break;
						}

						// get_width 
						while((*fmt)>='0' && (*fmt)<='9') {
							char tmp[2]; tmp[0]=*fmt; tmp[1]='\0';
							strcat(width,tmp);
							fmt++;
						}
						if(*fmt=='*') {
							width[0]='*'; width[1]='\0';
							fmt++;
						}

/*						// get_precision
						if(*fmt=='.') {
									while((*fmt)>='0' && (*fmt)<='9') {
											char tmp[2]; tmp[0]=*fmt; tmp[1]='\0';
											strcat(precision,tmp);
											fmt++;
									}
									if(*fmt=='*') {
											precision[0]='*'; precision[1]='\0';
											fmt++;
									}
						}
							
						// get_length
						switch(*fmt) {
							case 'h': length='h'; fmt++; break;
							case 'l': length='l'; fmt++; break;
							case 'L': length='L'; fmt++; break;
							default: break;
						}
						
*/


							if(*fmt== 'd'){
								int a=va_arg(ap,int);
								char str[20]; str[0]='\0';
								parse_str(a,str);
								int w=0;
								
								if(width[0]!='\0'){
												if(width[0]!='*') {
													parse_int(width,&w); 				
												}
								}
								
								if(myflags!='\0') {
												if(myflags==' ') {
														strcat(out," ");
														out++;
												}
												else if(myflags=='+') {
														if(a>0) {
															strcat(out,"+");
															out++;
														}
												}
								
								}

								if(w>strlen(str)) {
									char tmp[2]; tmp[1]='\0';
									if(myflags=='0')
													tmp[0]='0';
									else
													tmp[0]=' ';
									for(int i=w;i>strlen(str);i--) {
											strcat(out,tmp);
											out++;
									}

								}


								/*add +*/
								strcat(out,str);
								out+=strlen(str);
								fmt++;
								continue;
							}
							if(*fmt== 's'){

								char *s=va_arg(ap,char *);
								strcat(out,s);
								out+=strlen(s);
								fmt++;
								continue;
							}
					}
				}
  return strlen(out);
}

		
void parse_str(int a,char str[]){
				char b;
				bool isneg=false; bool isnegmax=false;
				char str2[20];
				if(a==0)  {
						str[0]='0'; str[1]='\0';
						return;
				} 
				else if(a <0) {
						isneg=true;
						int b=-a;
						if(b==a){
								isnegmax=true;
								a=a+1;
						} 
						a=-a;
				}
				int i=0;
				while(a!=0) {
						b='0'+a%10;
						str2[i]=b;
						a=a/10;
						i++;
				}
				if(isneg) {
						if(isnegmax) {
							str2[0]+=1;				
						}
						str2[i]='-';
						i++;
					}
				
				str2[i]='\0';
				for(int j=0;j<i;j++)
							str[j]=str2[i-j-1];
				str[i]='\0';
				return ;
}


void parse_int(char *str ,int * a) {
	*a=0; int base=1; int b[20]; int length=strlen(str);
	for(int i=0;i<length;i++,str++) 
					b[length-i-1]=(*str)-'0';
	for(int i=0;i<length;i++) {
		(*a)+=(base*b[i]);			
		base*=10;				
	}
	return;
}


int sprintf(char *out, const char *fmt, ...) {
		va_list ap;
		va_start(ap,fmt);
		return vsprintf(out,fmt,ap);
  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
