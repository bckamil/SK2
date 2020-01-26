#include <iostream>
#include <ctime>
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
#define PORT 8085
#define BUFOR_SIZE 50
#define HP 5
#define HASLA_W_PULI 3

const string hasla[]={"elo mordo","wiecej niz jedno zwierze to", "test"};

void sending_message(int id, string tresc, int len){
	write(id,tresc.c_str(),len);
}

class user{
	private:
	int socket_id;
	int score;
	int hp;
	bool ready;
	public:
	user(){
		this->socket_id=-1;
		this->score=-1;
		this->hp = -1;
		this->ready = false;
	}
	user(int socket_id){
		this->socket_id = socket_id;
		this->score = 0;
		this->hp = HP;
		this->ready = false;
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
		this->score=score;
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
	bool get_alive(){
		if(this->hp>0){
			return true;
		}
		else{
			cout << "Nie zyjesz\n";
			return false;
		}
	}
	void set_ready(bool ready){
		this->ready = ready;
	}
	bool get_ready(){
		return this->ready;
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
	pthread_mutex_t password_mutex;  
    public:
	room(){
		this->user_list = new user[MAX_PLAYERS];
		this->password = "haslo";
		this->revealed_password = "_____";
		this->password_len=5;
		this->room_alive=false;
		this->game_alive=false;
		pthread_mutex_init(&password_mutex,NULL);
	}
	void end(){
		int max=0;
		for(int i=0;i<MAX_PLAYERS;i++){
			cout<<user_list[i].get_socket_id()<< "ID przed end game\n";
		}
		for(int i=0;i<MAX_PLAYERS;i++){
			if(this->user_list[i].get_socket_id()>0 && this->user_list[i].get_ready()){
				if(max<user_list[i].get_score()){
					max = user_list[i].get_score();
				}
			}
		}
		for(int i=0;i<MAX_PLAYERS;i++){
			if(this->user_list[i].get_socket_id()>0){
				if(max==user_list[i].get_score()){
					string temp = "Wygrales\n";
					int id = this->user_list[i].get_socket_id();
					sending_message(id,temp,temp.length());
				}
				else{
					string temp = "Unlucky\n";
					int id = this->user_list[i].get_socket_id();
					sending_message(id,temp,temp.length());
				}
			}
		}
		for(int i=0;i<MAX_PLAYERS;i++){
			if(this->user_list[i].get_socket_id()>0){
				user_list[i].set_ready(false);
				user_list[i].set_score(0);
				user_list[i].set_hp(HP);
			}
		}
		for(int i=0;i<MAX_PLAYERS;i++){
			cout<<user_list[i].get_socket_id()<< "ID po end game\n";
		}
		this->game_alive=false;
		cout<<"koniec\n";
	}
	void start(){
		srand (time(NULL));
		int secret = rand() % HASLA_W_PULI;
		this->password=hasla[secret];
		cout<<password<< " -- haslo\n";
		cout<<password.length()<< " -- ma tyle znakow";
		this->password_len=password.length();
		this->revealed_password = this->password;
		for(int i = 0; i< password_len ; i++){
			if(password[i]>='a' && password[i]<='z'){
				revealed_password[i]='_';
			}
		}
		send_game();
		cout<<revealed_password<< " -- odkryte haslo\n";
	}
	int check_letter(char letter, int id){
		pthread_mutex_lock(&(this->password_mutex)); 
		int count = 0;
		cout<<id<<" identyfikator\n";
		cout<<user_list[id].get_socket_id()<<" socket\n";
		for(int i=0; i< this->password_len;i++){
			if(revealed_password[i]==letter){
				this->user_list[id].remove_hp(1);
				this->user_list[id].add_score(-5);
				cout<<user_list[id].get_score()<<"minus score\n";
				send_game();
				pthread_mutex_unlock(&(this->password_mutex)); 
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
		
		bool compere = true;
		for(int i=0; i< this->password_len;i++){
			if(password[i]==revealed_password[i]){
			}
			else{
				compere = false;
				break;
			}
		}
		if(compere){
			this->game_alive = false;
			end();
		}
		send_game();
		pthread_mutex_unlock(&(this->password_mutex)); 
		return count;
	}
	int check_password(string guess, int id){
		pthread_mutex_lock(&(this->password_mutex)); 
		if(guess == password){
			cout<<"trafiony\n";
			int counter=0;
			for(int i=0; i< this->password_len;i++){
				if(revealed_password[i]=='_'){
					counter +=1;
				}
			}
			revealed_password=password;
			cout<<counter<<"trafionych\n";
			this->user_list[id].add_score(10*counter);
			send_game();
			end();
		}
		else{
			this->user_list[id].add_score(-5);
			this->user_list[id].remove_hp(1);
			cout<<"pudlo\n";
		}
		pthread_mutex_unlock(&(this->password_mutex));
		return 1;
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
	void send_game(){
		for(int i = 0; i<MAX_PLAYERS; i++){
			if(this->user_list[i].get_socket_id()>0){
				string temp;
				temp.append(revealed_password);
				temp.append(";");
				temp.append(to_string(this->user_list[i].get_hp()));
				temp.append(";");
				temp.append(to_string(this->user_list[i].get_score()));
				temp.append("\n");
				cout<< temp << " - wiadomosc\n";
				sending_message(user_list[i].get_socket_id(),temp,temp.length());
			}
		}
	}
	bool get_game_alive(){
		return this->game_alive;
	}
	void ready_player(int id){
		cout<<id<<"id\n";
		this->user_list[id].set_ready(true);
		int counter = 0;
		int players = 0;
		for(int i=0;i<MAX_PLAYERS;i++){
			if(this->user_list[i].get_socket_id()>0){
				players +=1;
				cout<<user_list[i].get_socket_id()<<"id gracza\n";
			}
		}
		for(int j=0;j<MAX_PLAYERS;j++){
			if(this->user_list[j].get_ready()){
				counter +=1;
			}
		}
		cout<<players<<"graczy\n";
		cout<<counter<<" counter gotowych graczy\n";
		if(counter>1&&counter==players){
			this->game_alive=true;
			start();
		}
	}
	void set_game_alive(bool state){
		this->game_alive=state;
	}
	int add_user(user user){
		int add = 0;
		for(int i =0; i<MAX_PLAYERS;i++){
			if(this->user_list[i].get_socket_id()==-1){
				this->user_list[i] = user;
				cout<<i<<"id \n";
				add = i;
				return add;
			}
		}
		return add;
	}
	void remove_user(int id){
		user temp_user;
		this->user_list[id] = temp_user;
		int counter = 0;
		for(int i=0;i<MAX_PLAYERS;i++){
			if(this->user_list[i].get_socket_id()>0){
				counter +=1;
			}
		}
		if(counter<2&&this->game_alive){
			end();
		}
		cout<<counter<<" zostalo w pokoju \n";
		if(counter==0){
			this->room_alive=false;
			cout<<"zamknieto pokoj\n";
		}
	}
	bool get_user_alive(int id){
		return this->user_list[id].get_alive();
	}
};
struct thread_data{
	int connection_socket_descriptor; 
	int * connection_descriptor_array;
	room * room_list;
	int room_index;
	int player_index;
	bool * server_status;
	pthread_mutex_t room_list_mutex;  
	pthread_mutex_t connection_descriptor_mutex; 
}; 
char *readData(int fd,thread_data *t_data, bool *connected){

	char * buffor = new char[BUFOR_SIZE];
    char * temp = new char[2];
    int fail;
    memset(buffor,'\0',sizeof(char)*BUFOR_SIZE-1);
    memset(temp,'\0',2);
    do{
        fail=read(fd,temp,1);
        if(fail<0)
        {
            cout<< "Błąd przy próbie odczytu wiadomosci\n";
			
            pthread_exit(NULL);
        }
		if(fail==0){
			*connected=false;
			break;
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
			cout << t_data->player_index<<" connect po zapisaniu\n";
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
        buffor = readData(this_data->connection_socket_descriptor,this_data, &connected);
		if(!buffor){
			break;
		}
		
		if(!strncmp(buffor,"join", 4) && !room){
			int id = buffor[5] - '0';
			int result;
			string temp = "Error\n";
			pthread_mutex_lock(&(this_data->room_list_mutex)); 
			result = conecting_to_room(this_data, id);
			if(result){
				room = true;
				temp = to_string(id);
				temp.append("\n");
				this_data->room_index=id;
				std::cout<<this_data->player_index<<"\n";
			}
			sending_message(this_data->connection_socket_descriptor,temp,temp.length());
			pthread_mutex_unlock(&(this_data->room_list_mutex)); 
		}
		if(!strncmp(buffor,"create", 6) && !room){
			bool result;
			string temp = "Error\n";
			pthread_mutex_lock(&(this_data->room_list_mutex)); 
			result = creating_room(this_data, buffor);
			if(result){
				temp=to_string(this_data->room_index);
				temp.append("\n");
				write(1, buffor, BUFOR_SIZE);
				room = true;
			}
			sending_message(this_data->connection_socket_descriptor,temp,temp.length());
			pthread_mutex_unlock(&(this_data->room_list_mutex)); 
		}
		if(!strncmp(buffor,"send", 4)&& room && this_data->room_list[this_data->room_index].get_game_alive()&& this_data->room_list[this_data->room_index].get_user_alive(this_data->player_index)){
			char letter = buffor[5];
			this_data->room_list[this_data->room_index].check_letter(letter, this_data->player_index);
		}
		if(!strncmp(buffor,"ready", 5) && room){
			this_data->room_list[this_data->room_index].ready_player(this_data->player_index);
			cout << "ready\n";
		}
		if(!strncmp(buffor,"leave", 5) && room){
			pthread_mutex_lock(&(this_data->room_list_mutex)); 
			this_data->room_list[this_data->room_index].remove_user(this_data->player_index);
			this_data->player_index=-1;
			this_data->room_index=-1;
			cout << "usunieto\n";
			pthread_mutex_unlock(&(this_data->room_list_mutex)); 
			room=false;
			string temp = "1\n";
			sending_message(this_data->connection_socket_descriptor,temp,temp.length());
		}
		if(!strncmp(buffor,"guess", 5) && room && this_data->room_list[this_data->room_index].get_game_alive() && this_data->room_list[this_data->room_index].get_user_alive(this_data->player_index)){
			string temp = "";
			int counter = -2;
			for(int i = 5;i<BUFOR_SIZE;i++){
				if(buffor[i]!='\0'){
				counter+=1;
				}
			}
			temp.append(buffor,6,counter);
			//cout<<temp<<" koniec\n";
			this_data->room_list[this_data->room_index].check_password(temp, this_data->player_index);
		}

		delete buffor;
	}
	cout << "Umarlem\n";
	close(this_data->connection_socket_descriptor);
	if(this_data->room_index>-1){
		this_data->room_list[this_data->room_index].remove_user(this_data->player_index);
	}
	delete this_data;
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
	pthread_mutex_t room_list_mutex;  
	pthread_mutex_t connection_descriptor_mutex;  
	pthread_mutex_init(&room_list_mutex,NULL);
	pthread_mutex_init(&connection_descriptor_mutex,NULL);
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
		t_data->room_list_mutex=room_list_mutex;
		t_data->connection_descriptor_mutex=connection_descriptor_mutex;

        pthread_create(&threadid, NULL,client_handler, (void *)t_data);
	}
	close(fd);
    pthread_mutex_destroy(&room_list_mutex);
	pthread_mutex_destroy(&connection_descriptor_mutex);
	delete []room_list;
    delete []connection_descriptor_array;
    delete server_status;
	return(0);
}

