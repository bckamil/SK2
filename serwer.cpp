#include <iostream>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <error.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <mutex>

using namespace std; 

#define MAX_PLAYERS 10
#define MAX_ROOMS 10
#define PORT 8084
#define BUFOR_SIZE 50
#define HP 5
class user{
	private:
	int socket_id;
	int score;
	int hp;
	public:
	user(){
		this->socket_id=-1;
		this->score=-1;
		this->hp = -1;
	}
	user(int socket_id){
		this->socket_id = socket_id;
		this->score = 0;
		this->hp = 5;
	}
	int get_socket_id()
	{
		return this->socket_id;
	}
	void set_socket_id(int socked_id){
		this->socket_id=socked_id;
	}
	int get_score()
	{
		return this->score;
	}
	void set_score(int score){
		this->socket_id=score;
	}
	void add_score(int good){
		this->score+=good;
	}
	int get_hp()
	{
		return this->hp;
	}
	void set_hp(int hp){
		this->hp=hp;
	}
	void remove_hp(int bad){
		this->hp -= bad;
	}
};
// klasa pokoj
class room 
{ 
    private:
	user *user_list;
	string password;
	string revealed_password;
	int password_len;
	bool room_alive;
	bool game_alive;
    public:
	room(){
		this->user_list = new user[MAX_PLAYERS];
		this->password = "haslo";
		this->revealed_password = "_____";
		this->password_len=5;
	}
	int check_letter(char letter, int id){
		int count = 0;
		cout<<id<<" identyfikator\n";
		cout<<user_list[id].get_socket_id()<<" socket\n";
		for(int i=0; i< this->password_len;i++){
			if(revealed_password[i]==letter){
				this->user_list[id].remove_hp(1);
				this->user_list[id].add_score(-5);
				cout<<user_list[id].get_score()<<"minus score\n";
				return count;
			}
		}
		for(int i=0; i< this->password_len;i++){
			if(password[i]==letter){
				revealed_password[i]=letter;
				count +=1;
			}
		}
		if(count>0){
			this->user_list[id].add_score(10*count);
			cout<<user_list[id].get_score()<<"dodane \n";
		}
		else{
			this->user_list[id].remove_hp(1);
				this->user_list[id].add_score(-5);
				cout<<user_list[id].get_score()<<"pudlo \n";
		}
		return count;
	}
	string get_password(){
		return this->password;
	}
	void set_password(string password){
		this->password = password;
	}
	string get_revealed_password(){
		return this->revealed_password;
	}
	void set_revealed_password(string revealed_password){
		this->revealed_password = revealed_password;
	}
	bool get_room_alive(){
		return this->room_alive;
	}
	void set_room_alive(bool state){
		this->room_alive=state;
	}
	bool get_game_alive(){
		return this->game_alive;
	}
	void set_game_alive(bool state){
		this->game_alive=state;
	}
	int add_user(user user){
		int add = 0;
		for(int i =0; i<MAX_PLAYERS;i++){
			if(this->user_list[i].get_socket_id()==-1){
				this->user_list[i] = user;
				cout<<i<<"\n";
				add = i;
				return add;
			}
		}
		return add;
	}
};
struct thread_data{
	int connection_socket_descriptor; 
	int * connection_descriptor_array;
	room * room_list;
	int room_index;
	int player_index;
	bool * server_status;
}; 
char *readData(int fd){

	char * buffor = new char[BUFOR_SIZE];
    char * temp = new char[2];
    int fail;
    memset(buffor,'\0',sizeof(char)*BUFOR_SIZE-1);
    memset(temp,'\0',2);
    do{
        fail=read(fd,temp,1);
        if(fail==-1){
			error(1,errno, "read failed on descriptor %d", fd);
		} 
        strcat(buffor,temp);
    }while(strcmp(temp,"\n"));
    delete temp;
    return buffor;
};
bool conecting_to_room(thread_data *t_data,int id){
	bool join = false;
	int result;
	user temp_user(t_data->connection_socket_descriptor);
	if(!t_data->room_list[id].get_game_alive()&&t_data->room_list[id].get_room_alive()){
		result = t_data->room_list[id].add_user(temp_user);
		if(result>0){
			join = true;
			cout << result<<"connect\n";
			t_data->room_index=id;
			t_data->player_index=result;
		}
	}
	return join;
}
bool creating_room(thread_data *t_data,char *buffor){
	bool create = false;
	for(int i = 0; i<MAX_ROOMS; i++){
		if(!t_data->room_list[i].get_room_alive()){
			t_data->room_list[i].set_room_alive(true);
			user temp_user(t_data->connection_socket_descriptor);
			t_data->room_list[i].add_user(temp_user);
			t_data->room_index=i;
			t_data->player_index=0;
			create = true;
			break;
		}
	}
	return create;
}
void * client_handler(void  *t_data)
{
	pthread_detach(pthread_self());
	bool connected = true;
	bool room = false;
	char * buffor;
	thread_data *this_data = (thread_data*)t_data;
	while(connected && *(this_data->server_status)){
        buffor = readData(this_data->connection_socket_descriptor);
		if(!buffor){
			break;
		}
		
		if(!strncmp(buffor,"join", 4) && !room){
			int id = buffor[5] - '0';
			int result;
			result = conecting_to_room(this_data, id);
			if(result){
				room = true;
				this_data->room_index=id;
				this_data->player_index=result;
				//std::cout<<this_data->player_index<<"\n";
			}
		}
		if(!strncmp(buffor,"create", 6) && !room){
			bool result;
			result = creating_room(this_data, buffor);
			if(result){
				write(1, buffor, BUFOR_SIZE);
			}
			room = true;
		}
		if(!strncmp(buffor,"send", 4)){
			char letter = buffor[5];
			int result;
			result = this_data->room_list[this_data->room_index].check_letter(letter, this_data->player_index);
		}

	}
	return (void *)0;
}
int main(int argc, char ** argv) {
	sockaddr_in myAddr {};
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons((uint16_t)PORT);

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	    if(fd == -1){
		perror("socket failed");
		return 1;
	    }
	int fail = bind(fd, (sockaddr*) &myAddr, sizeof(myAddr));
    	if(fail){
		perror("bind failed");
		return 1;
    	}
    
    	fail = listen(fd, 1);
    	if(fail){
		perror("listen failed");
		return 1;
    	}
	//tworzenie zmiennych do struktury
	int * connection_descriptor_array = new int [MAX_ROOMS*MAX_PLAYERS];
	memset(connection_descriptor_array,-1,MAX_ROOMS*MAX_PLAYERS*sizeof(int));
	room * room_list = new room[MAX_ROOMS]; 
	bool *server_status = new bool;
	*server_status = true;
	while(*server_status){
		int clientFd = accept(fd, nullptr, nullptr);
        	if(clientFd == -1){
            		perror("accept failed");
            		return 1;
        	}
		pthread_t threadid;
		thread_data* t_data=new thread_data;
		t_data->connection_socket_descriptor = clientFd;
		t_data->connection_descriptor_array=connection_descriptor_array;
		t_data->room_list=room_list;
		t_data->room_index=-1;
		t_data->server_status=server_status;

        pthread_create(&threadid, NULL,client_handler, (void *)t_data);
	}
	close(fd);
	return(0);
}

