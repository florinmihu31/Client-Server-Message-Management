Tema 2 - PC
Mihu Florin - 324CC

    In fisierul server.cpp avem socketi pentru conexiunile TCP SI UDP, 
realizandu-se conexiuni intre acestea si server. Pe socketul de TCP ascultam 
comenzi de la clientii TCP. Intr-o bucla infinita se parcurge multimea de file 
descriptori si se verifica daca s-a primit o comanda de la tastatura, daca s-a 
conectat un client TCP, daca s-a primit un mesaj de la un client UDP sau daca 
s-a primit o comanda de abonare/dezabonare la un topic al unui client TCP.
    De la tastatura se poate primi doar comanda "exit", in cazul altor erori 
serverul se opreste doar in momentul cand sunt probleme la socketi, port, 
realizarea legaturii sau un numar invalid de argumente in linia de comanda.   
    Daca primim o cerere de conectare, atunci verificam daca utilizatorul se 
afla sau nu in vectorul de clienti. Daca se afla si este activ atunci i 
transmite faptul ca deja este online si i se refuza conexiunea. In schimb, daca 
este offline, atunci i se afiseaza toate mesajele primite cat a fost inactiv. 
    Daca primim un mesaj de la UDP, atunci il decodificam si il impachetam 
intr-o structura de tip message_to_client, atasand, pe langa continut, ip-ul si 
portul. In cele din urma, parcurgem lista de clienti si trimitem mesajul celor 
care sunt abonati la topicul din mesaj.
    Daca primim comenzi de subscribe/unsubscribe, receptionam comanda si apoi 
facem cast mesajului primit la o structura de tip message_to_server. Daca 
actiunea este de tipul SUBSCRIBE, atunci adaugam clientului curent topicul din 
mesaj in lista de topicuri abonate, altfel daca mesajul este de tip 
UNSUBSCRIBE, atunci gasim clientul curent si topicul corespunzator in lista sa 
de topicuri, iar apoi il eliminam.


    In fisierul subscriber.cpp, ne conectam folosind socketul la server pe 
portul dat ca parametru si trimitem serverului id-ul utilizatorului. Intr-o 
bucla infinita selectam din multimea de file descriptori socketul potrivit si 
verificam daca primim mesaj de la tastatura sau mesaj de la server.
    De la tastatura putem primi comenzi de subscribe/unsubscribe/exit. Daca 
primim mesaj de subscribe/unsubscribe verificam daca este valida si o 
impachetam intr-un mesaj pentru server, pe care il trimitem in final.
    Daca primim mesaj de la server, il castam intr-o structura de tip 
message_to_client. In functie de tipul de date primit afisam mesajul pe ecranul 
clientului daca este unul corect, in caz contrar afisam o eroare.
