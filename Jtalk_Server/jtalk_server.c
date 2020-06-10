/*
	Salih Can YILMAZ - 160101005
	Canberk İLERİ    - 150101007

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "socketfun.h"
#include <pthread.h>
#include "jrb.h"
#include "jval.h"
#include "time.h"
#include <signal.h>


#define NAMELEN 35

//GLOBAL VARIABLES
JRB conns;//Baglantilari tutmak icin kullanilan JRB agaci
pthread_mutex_t lock;//JRB agaci ile ilgili islemler icin mutex
pthread_mutex_t siglock;//Sigpipe errorflagi ile ilgili islemler icin mutex
int sigerr;

//Baglantilari kontrol etmek icin gerekli bir struct
typedef struct c_control{//connection control struct
	pthread_t tid;	
	int fd;
	char *name;
	int active;
	time_t tconn;
	time_t tdisconn;
	time_t ttalked;

}* C_control;


//FUNCTION HEADERS
C_control  new_Connection();
void sigpipe_handler(int dummy);
void logout(C_control conn);
void *jtalk_console();
void *simpcat(void *f);
void *serve_connection(void *f);
void *seek_connections(void *s);




main(int argc, char **argv)
{
	int sock, i;
	pthread_t acctid;//seek connection icin thread id
	pthread_t jtctid;//jtalk console icin thread id

	void *status;

  if (argc != 3) {
    fprintf(stderr, "usage: jtalk_server host port\n");
    exit(1);
  }

	conns = make_jrb();

  sock = serve_socket(argv[1], atoi(argv[2]));

	signal(SIGPIPE, sigpipe_handler);						
	 

  if (pthread_create(&acctid, NULL, seek_connections, &sock) < 0) {
		perror("pthread_create");
		exit(1);
	}
 
   

	//Jtalk Console icin thread olustur
  if (pthread_create(&jtctid, NULL, jtalk_console, NULL) < 0) {
		perror("pthread_create");
		exit(1);
	}
	//Jtalk Console'u  bekle
	pthread_join(jtctid, &status);
	
	

  close(sock);


  return 1;



}


//C_control constructor
C_control  new_Connection()
{
  C_control  conn;
  conn = (C_control) malloc (sizeof(struct c_control)); 
  conn->fd = 0;
	conn->active = 1;
  return  conn;
}



//(Detayli aciklama simpcat fonksiyonunda mevcut)
void sigpipe_handler(int dummy)
{
  pthread_mutex_lock(&siglock);
  sigerr = 1;
	pthread_mutex_unlock(&siglock);
	signal(SIGPIPE, SIG_IGN);						

}

//Cikis yapan kullanicinin agactaki baglanti structini duzenler.
void logout(C_control conn)
{
	char* logoutmsg;
	logoutmsg = " has logged out\n";
	conn->active = 0;
	close(conn->fd);
	sigerr = 0;


	
	time(&conn->tdisconn);

  
	write(1,conn->name,strlen(conn->name));
	write(1,logoutmsg,strlen(logoutmsg));	




}


/*
		Jtalk console mainden fork edilen bir thread'de calisir. Sunucudaki mevcut baglant durumlarini izlemek ve
	sunucuyu kapatmak icin bir arayuz olusturur. 
		Eger kullanici yanlis bir komut girerse HELP mesajiyla kullaniciya
	dogru komutlar ve ne ise yaradiklari bildirilir.
		TALKERS komutu baglanti agacini gezerek aktif olan baglantilari
	ve bu baglantilarin en son mesaj gonderdikleri zamani yazdirir.
		ALL komutu serverdaki aktif olan ve olmayan tum baglantilari, ilk baglanma zamanlarini,
	cikis yapma zamanlarini, ve en son mesaj attiklari zamani yazdirir.
		EXIT komutu ise serverin calismasini sonlandirir.
		TALKERS ve ALL komutlari  agacla ilgili islemlerini mutex'i kilitleyerek yaparlar.
*/


void *jtalk_console()
{
	int done;
	char inpt[20];
	char* welcomemsg;
	char* helpmsg;
	char* talkhead;
	char* allhead;
	char* tab;
	JRB temp;
	C_control conn;	
	char* tmptime;
	char* empty;
	char* dflt;

	welcomemsg = "Welcome to JTALK_SERVER\nFor Help: HELP\n";
	helpmsg = "For all connections 		: ALL\nFor all active connections 	: TALKERS\nFor exit 			: EXİT\n";	
	talkhead = "User\t\t\tLast Talked\n";
	allhead = "User\t\t\tLogged In\t\t\tLogged Out\t\t\tLast Talked\n";
	tab = "\t\t\t"; 
	empty = "------------------------";
	dflt = "Thu Jan  1 02:00:00 1970\n";
	write(1,welcomemsg,strlen(welcomemsg));
	
	done = 0;
	while(done == 0)
	{
		scanf("%s", inpt);
		if(strcmp(inpt,"TALKERS") == 0)
		{
			pthread_mutex_lock(&lock);
			write(1,talkhead,strlen(talkhead));
			jrb_traverse(temp, conns)
			{		
				conn = (C_control) temp->val.v;
				if(conn->active == 1)
				{
					tmptime = ctime(&conn->ttalked);
					write(1,conn->name,strlen(conn->name));
					write(1,tab,strlen(tab));
					write(1,tmptime,strlen(tmptime));
				}		
			}
			pthread_mutex_unlock(&lock);	
		}
		else if(strcmp(inpt,"ALL") == 0)
		{
			pthread_mutex_lock(&lock);
			write(1,allhead,strlen(allhead));
			jrb_traverse(temp, conns)
			{								
				conn = (C_control) temp->val.v;
				tmptime = ctime(&conn->tconn);
				write(1,conn->name,strlen(conn->name));
				write(1,tab,strlen(tab));
				write(1,tmptime,strlen(tmptime)-1);
				write(1,tab,strlen(tab)-2);
				tmptime = ctime(&conn->tdisconn);
			  if(strcmp(tmptime, dflt) != 0 )
				{
					write(1,tmptime,strlen(tmptime)-1);
					write(1,tab,strlen(tab)-2);
				}
				else
				{
					write(1,empty,strlen(empty)-1);
					write(1,tab,strlen(tab)-1);
				}




		
				tmptime = ctime(&conn->ttalked);
				write(1,tmptime,strlen(tmptime));
			}		
			pthread_mutex_unlock(&lock);		
		}
		else if(strcmp(inpt,"EXIT") == 0)
		{
			done = 1;
		}		
		else  //HELP			
		{
			write(1,helpmsg,strlen(helpmsg));
		}
		
	}
	
	pthread_exit(NULL);
			
}


/*
	simpcat(ismini degistirmedik)'in temel gorevi baglantinin fd'sinden gelen her bir karakteri
	baglanti agacindaki tum baglanti fd'lerine gondermektir.
	Bir baglanti acildiginda gonderilen ilk veriler "kullanici ismi:" seklinde olacagindan bu baglantiyi
	tutmak icin : karakteri okunana kadar gelen karakterleri isim olarak kaydeder ve yeni bir baglanti kontrol
	struct'i olusturup fd ile birlikte baglanti agacina ekler. Baglanti agaciyla ilgili tum islemlerde global
	olarak tutulan mutek kilidi kitlenir ve islem bitince kilit acilir.

	Eger bir kullanici cikis yapmis ise bir sonraki yazma isleminde onun dosyasi kapali olacagindan SIGPIPE
	sinyali verilecektir. Boyle bir durumda programin sorun olmadan calismaya devam etmesi icin sigpipe handler
	yazilmistir. sigpipe handler globalde tutulan error flagini 1 yapar daha sonra SIGPIPE sinyalini ignore eder
	Daha sonra simpcat icinde error flagin 1 oldugu fark edilirse o baglantinin artik aktif olmadigi anlasilir ve
	baglanti kontrol yapisindaki aktif alani 0 yapilir. Simpcat sadece aktif olan baglantilara yazma yapacagindan
	bundan sonrasi icin olasi problemler ortadan kaldirilmis olur.
*/
void *simpcat(void *f)
{
	char c;//gelen karakteri okumak icin
	char name[NAMELEN];
	int fd, *fdp;
	int i = 0;
	int nsize = 0;
	int namecheck = 0;
	C_control conn, tmpconn;
	char *errmsg;
	JRB temp; 


	errmsg = "Isim 35 karakterden daha uzun olamaz!\n";

	fdp = (int *) f;
  fd = *fdp;

	conn = new_Connection();
	conn->fd = fd;
	time(&conn->tconn); 
	pthread_mutex_lock(&lock);//Agaca ekleme yapilacak mutexi kilitle
	jrb_insert_int(conns,fd,new_jval_v((void *) conn));
	pthread_mutex_unlock(&lock);//Agaca ekleme yapildi mutex kilidini ac

	
  while (1) {
	
			if(read(fd, &c, 1) == 1)
			{	
				time(&conn->ttalked);
			
					
				if(namecheck == 0)
				{
					if(nsize < NAMELEN)
					{
						if(c < 30)//Control karakterlerini at
						{
							
						}
						else
						{
							if(c != ':')
							{
								nsize++;
								//name = (char *) malloc(sizeof(char)*(nsize));
								name[nsize-1] = c;  							
								
								
							}
							else
							{

								conn->name = (char *) malloc(sizeof(char)*(nsize));
								conn->name = strdup(name);

							
								namecheck = 1;
							}
						}
					}
					else
					{
						write(fd, errmsg, strlen(errmsg));
					}
					
				}

		
				
					pthread_mutex_lock(&lock);	//Agacta gezilip yazma yapilacak mutexi kilitle
					jrb_traverse(temp, conns)
					{
						tmpconn = (C_control) temp->val.v;
						if(tmpconn->active == 1)
						{
							
							write(tmpconn->fd,&c,1);
							pthread_mutex_lock(&siglock);	
							if(sigerr == 1)
							{
								signal(SIGPIPE, sigpipe_handler);						
								logout(tmpconn);							
							}
							pthread_mutex_unlock(&siglock);	
						}						
					}		
					pthread_mutex_unlock(&lock);	//mutex kilidini ac		
					if(conn->active == 0 )
					{
						pthread_exit(NULL);
					}
	
			
			}
		

	

	}

	

	return NULL;
}


//gelen file descriptor'i simpcat threadine gonder ve cik
void *serve_connection(void *f) 
{
  
	int fd, *fdp, i;
	Jval j;
	pthread_t tid;
	


	fdp = (int *) f;
  fd = *fdp;
	
	
			
		
		if (pthread_create(&tid, NULL, simpcat, fdp) < 0) {
			perror("pthread_create");
			exit(1);
		}
	


  
	

		
		return NULL;
	
	
	
}

//Gelen baglanti isteklerini dinleyip bu isteklerin herbiri icin yeni bir thread olusturan fonksiyon
void *seek_connections(void *s)
{
	int sock, *sockp, fd, i;
	pthread_t tid;
	
	


	sockp = (int *) s;
  sock = *sockp;

	pthread_t tidlist[500], temptid;
	void *status;

	


  i = 0;
	while (1) {//sonsuz dongu icinde surekli connection bekler
		 
	
  	fd = accept_connection(sock);
	
		
    temptid = tidlist[i];

		


		if (pthread_create(&temptid, NULL, serve_connection, &fd) < 0) {//connection gelirse serve_connection threadi olustur
			perror("pthread_create");
			exit(1);
		}
              
		//pthread_join(temptid, &status);
		i++;
		
    
  }  
}


