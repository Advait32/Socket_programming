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
#include<fstream>

using namespace std;

int main(int argc, char *argv[]){
    ofstream file;
    file.open("receiver.txt");
    int sockfd_l, sockfd_s;

    struct addrinfo hint_s ,hint_l ;
    struct addrinfo *servinfo_l, *servinfo_s, *p_l, *p_s;
    int rv_l, rv_s;
    int numbytes_l, numbytes_s;
    struct sockaddr_storage their_addr_l;
    char buf[100];
    socklen_t addr_len_l;

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
    int xy = 0;
    freeaddrinfo(servinfo_l);
    addr_len_l = sizeof their_addr_l;

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
    string pdp_s(argv[3]); 
    double pdp = atof(pdp_s.c_str());

    int i = 1;
    while(true){
        if ((numbytes_l = recvfrom(sockfd_l, buf, 99 , 0,(struct sockaddr *)&their_addr_l, &addr_len_l)) == -1) {
            exit(1);
        }

        string str(buf);
        string sub = str.substr(7,str.length()-1);
        int ind = stoi(sub);
        cout << "received : \"Packet:" << ind << "\""<<endl;
        file << "received : \"Packet:" << ind << "\""<<endl;
        string sub1 = "Acknowledgement:";
        string sub2 = to_string(ind+1);

        if(ind != i){
            sub2 = to_string(ind);
            string st = sub1 + sub2;
            
            char c[st.size()+1];
            strcpy(c,st.c_str());
            cout << "Sent:\"Acknowledgement:"<< i<< "\""<<endl;
            file << "Sent:\"Acknowledgement:"<< i<< "\""<<endl;
            sendto(sockfd_s,c,strlen(c),0,p_s->ai_addr, p_s->ai_addrlen);
        }
        else{
            srand(unsigned(xy*time(NULL)));				//Dropping Packets with probability = pdp(packet drop probability).
            double f = (double)rand() / RAND_MAX;
            //cout << pdp << " " << f << endl;
            xy++;
            if(f > pdp){
                string st = sub1 + sub2;
                char c[st.size()+1];
                strcpy(c,st.c_str());
                sendto(sockfd_s, c, strlen(c), 0, p_s->ai_addr, p_s->ai_addrlen);
                cout << "Sent:\"Acknowledgement:"<< i+1<< "\""<<endl;
                file << "Sent:\"Acknowledgement:"<< i+1<< "\""<<endl;  
                i++;
                continue;
            }
            else{
                cout << "Dropped:\"Acknowledgement:"<< ind<< "\""<<endl; 
                file << "Dropped:\"Acknowledgement:"<< ind<< "\""<<endl; 
            }
        }
    }
    close(sockfd_l);
    close(sockfd_s);
file.close();
}