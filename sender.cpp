#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <iostream>
#include <string>
#include <cmath>
#include <random>
#include <sys/poll.h>
#include <sys/select.h>
#include<fstream>

using namespace std;

int main(int argc, char *argv[]){
    int sockfd_l, sockfd_s;
    fd_set readfds;
    struct timeval timeout;
    ofstream file;
    file.open("sender.txt");
    struct addrinfo hint_s ,hint_l ;
    struct addrinfo *servinfo_l, *servinfo_s, *p_l, *p_s;
    int rv_l, rv_s;
    int numbytes_l, numbytes_s;
    struct sockaddr_storage their_addr_l;
    char buf[100];

    memset(&hint_l, 0, sizeof hint_l);
    hint_l.ai_family = AF_INET;
    hint_l.ai_socktype = SOCK_DGRAM;
    hint_l.ai_flags = AI_PASSIVE;
    
    if((rv_l = getaddrinfo(NULL,argv[1],&hint_l,&servinfo_l)) != 0){
        return 1;
    }

    for(p_l = servinfo_l; p_l != NULL; p_l = p_l->ai_next){
        if((sockfd_l = socket(p_l->ai_family,p_l->ai_socktype,p_l->ai_protocol)) == -1){
            continue;
        }

        if(bind(sockfd_l, p_l->ai_addr, p_l->ai_addrlen) == -1){
            close(sockfd_l);
            continue;
        }

        break;
    }

    if(p_l == NULL){
        return 2;
    }

    freeaddrinfo(servinfo_l);

    memset(&hint_s, 0, sizeof hint_s);
    hint_s.ai_family = AF_INET;
    hint_s.ai_socktype = SOCK_DGRAM;
    hint_s.ai_flags = AI_PASSIVE;
    
    if ((rv_s = getaddrinfo(NULL, argv[2], &hint_s, &servinfo_s)) != 0) {
        return 1;
    }

    for(p_s = servinfo_s; p_s != NULL; p_s = p_s->ai_next){
        if((sockfd_s = socket(p_s->ai_family, p_s->ai_socktype, p_s->ai_protocol)) == -1){
            continue;
        }
        break;
    }

    if(p_s == NULL){
        return 2;
    }
    string st(argv[3]);
    int retransit_timer = stoi(st);

    string st2(argv[4]);
    int num_packet = stoi(st2);

    timeout.tv_sec = retransit_timer;
    timeout.tv_usec = 0;

    string mes_send = "Packet:";
    int nm = 1;
    char send[100];
    socklen_t addr_len_l = sizeof their_addr_l;

    string temp = "";
    while(nm < num_packet+1){
        string s = mes_send + to_string(nm);
        char send[s.length()+1];
        for(int j =0; j< s.length(); j++){
            send[j] = s[j];
        }
        send[s.length()+1] = '\0';

        int flag =0;
        time_t start = time(NULL), end;
        sendto(sockfd_s,send,strlen(send),0,p_s->ai_addr,p_s->ai_addrlen);            
        cout << "Sent:\"Packet:"<< nm << "\""<<endl;
        file << "Sent:\"Packet:"<< nm << "\""<<endl; 
        int x = setsockopt(sockfd_l,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout, sizeof timeout);
        if(x == 0){
            char buf[100];
            int y=recvfrom(sockfd_l,buf,99,0,(struct sockaddr *)&their_addr_l, &addr_len_l);
            if(y<0)
            {
                continue;
            }


            string str1(buf);
            string sub1 = str1.substr(16,str1.length()-1);

            int ind = stoi(sub1);          
            

            if(ind == nm+1)
            {
                nm = ind;
                timeout.tv_sec = retransit_timer;
                cout << "Received:\"Acknowledgement:"<< nm<< "\""<<endl;
                file << "Received:\"Acknowledgement:"<< nm<< "\""<<endl; 
            }
            else{
                cout << "Received:\"Acknowledgement:"<< ind<< "\" and ignored"<<endl;
                file << "Received:\"Acknowledgement:"<< ind<< "\" and ignored"<<endl; 
                timeout.tv_sec = difftime(time(NULL),start);   //Resuming Transmission in case of ignored acknowledgements
            }


        }
        else if(x != 0){
            continue;
        }
    }

file.close();
}