//
// Created by bisakh on 27/11/20.
//

#ifndef FLOWCONTROL_CRC_H
#define FLOWCONTROL_CRC_H

#include<iostream>

using namespace std;

string XOR(string encoded , string crc){

    int crclen =  crc.length();

    for ( int i = 0 ; i <= (encoded.length() - crclen) ; )			// performing bitwise xor operation
    {																
        for( int j=0 ; j < crclen ; j++)							
        {
            encoded[i+j] = encoded[i+j] == crc[j] ? '0' : '1' ;	
        }
        for( ; i< encoded.length() && encoded[i] != '1' ; i++) ;

    }

    return encoded;
}
const string crc32 = "100000100110000010001110110110111";

string CRCGEN(string word){
    string encoded = word;
    for(int i=0;i<crc32.length()-1;i++){
        encoded+="0";
    }
   encoded = XOR(encoded, crc32);
//   cout<<"CRC : "<<gen<<endl;
   return word + encoded.substr(encoded.length() - crc32.length() + 1);
}

int isCRCValid(string word){
    string msg = XOR(word, crc32 );
    for( char i : msg.substr(msg.length() - crc32.length() + 1))					//after performing xor , if the last few bits are zero then there's no error in transmission
        if( i != '0' )
        {
            cout<<"Error in communication "<<endl;						//if bits not zero ; ERROR IN TRANSMISSION
            return 0;
        }
    return 1;
}


#endif //FLOWCONTROL_CRC_H
