Vm1 = '131.247.2.242'
Vm2 = '72.36.112.72'
Vm3 = '210.32.181.184'
Vm4 = '192.12.33.102'
Vm5 = '133.69.32.133'
Vm6 = '210.75.225.60'
Vm = [Vm1,Vm2,Vm3,Vm4,Vm5,Vm6]

Client = Vm1
Server = Vm6

neib = {}
neib[Vm1]=[Vm2,Vm4]
neib[Vm2]=[Vm1,Vm3,Vm5,Vm6]
neib[Vm3]=[Vm2,Vm6]
neib[Vm4]=[Vm1,Vm5]
neib[Vm5]=[Vm2,Vm4,Vm6]
neib[Vm6]=[Vm2,Vm3,Vm5]

HOST = ''
PORT = 21568
BUFSIZ = 1024
ADDR = (HOST,PORT)
PUTFILE = 'pof.txt'

from os import popen
from random import randint
from socket import *
from threading import Thread
from time import ctime,time,sleep

RREQ = '1'
RREP = '2'
BC = '255.255.255.255'
BDAR = (BC,PORT)


se = 0

def getip():
    ip = os.popen("/sbin/ifconfig | grep 'inet addr' | awk '{print $2}'").read()
    ip = ip[ip.find(':')+1:ip.find('\n')]
    return ip

def parsedata(data):
    data = data.decode()
    return data.split(';')

def codedata(input_data):
    data = ';'.join(input_data)
    data = data.encode()
    return data

def send():
    global recieved
    global mutex
    mutex = Lock()
    #ip = getip()
    ip = '131.247.2.242'
    udpSerSock = socket(AF_INET,SOCK_DGRAM)
    #udpSerSock.setsockopt(SOL_SOCKET,SO_REUSEADDR,1)
    #udpSerSock.setsockopt(SOL_SOCKET,SO_BROADCAST,1)
    filename = 'send'+str(int(time()))+PUTFILE
    fr = open(filename,'w')
    dthash = 9909
    mysquencenum = 5080
    for j in range(100):
        #dthash = randint(0,10000)
        dthash += 1
        print(str(j)+'.message:'+str(dthash)+str(mysquencenum))
        cip = ip
        data = codedata([RREQ,cip,str(dthash),str(mysquencenum)])
        t = time()
        #print>>fr,'RREQ:time:'+str(t)+',message:'+str(dthash)
        print>>fr,'RREQ:time:'+str(t)+',message:'+str(dthash)+str(mysquencenum)
        

        if mutex.acquire(1):            
            recieved = False            
            mutex.release()            
        while True:            
            mysquencenum += 1
            print("send RREQ",dthash,mysquencenum)
            data = codedata([RREQ,cip,str(dthash),str(mysquencenum)])
            for nei in neib[ip]:
                udpSerSock.sendto(data,(nei,PORT))                
            sleep(3)
            if mutex.acquire(1):
                print("after 3s ",recieved)
                if recieved:
                    mutex.release()
                    break
                else:
                    pass
                print("mutex.release ",recieved)
                mutex.release()
                
    sleep(10)
    data = ''.encode()
    for nei in neib[ip]:
        udpSerSock.sendto(data,(nei,PORT))
    fr.close()



def listen():
    global recieved
    global mutex 
    mutex = Lock()
    #ip = getip()
    interest = []
    datacache = []
    ip = '131.247.2.242'
    nei = neib[ip]
    se = 0
    sendpktnum = 0
    udpSerSock = socket(AF_INET,SOCK_DGRAM)
    #udpSerSock.setsockopt(SOL_SOCKET,SO_REUSEADDR,1)
    #udpSerSock.setsockopt(SOL_SOCKET,SO_BROADCAST,1)
    udpSerSock.bind(ADDR)
    filename = 'listen'+str(int(time()))+PUTFILE
    fl = open(filename,'w')
    while True:
        #print('waiting for message...')
        data,addr = udpSerSock.recvfrom(BUFSIZ)
        if data:
            if addr[0] not in nei:
                continue
            data = parsedata(data)
            mk = data[0]
            if mk == RREQ:
                print('get RREQ from:',addr[0])
                cip = data[1]
                dthash = data[2]
                pktsequencenum = data[3]
                for dataj in datacache:
                    if dthash == dataj[0]:
                        dataincache = 1
                        se = 1
                        break
                if not se:
                    iin = 0
                    for intere in interest:
                        if dthash == intere[0] and pktsequencenum == intere[2]:
                            iin = 1
                    if iin:
                        pass
                    else:
                        interest.append((dthash,addr[0],pktsequencenum))
                        data = codedata([mk,cip,dthash,pktsequencenum])
                        
                        for neibh in nei:
			    if neibh != addr[0]:
                                udpSerSock.sendto(data,(neibh,PORT))
				sendpktnum += 1
                                print('send RREQ to:',neibh)
                else:
                    if (dthash,cip,pktsequencenum) in interest:
                        pass
                    else:
                        interest.append((dthash,cip,pktsequencenum))
                        data = [RREP,cip,ip,addr[0],dthash]
                        data = codedata(data)                        
                        udpSerSock.sendto(data,(addr[0],PORT))
			sendpktnum += 1
                        print('send RREP to:',addr[0])
            elif mk == RREP:
                cip = data[1]
                sip = data[2]
                nextip = data[3]
                dthash = data[4]
                print('recv RREP from:',sip)
                if nextip == ip:
                   # for neibh in nei:
                   #     print('forward RREP to',nextip)
                   #     udpSerSock.sendto(data,(neibh,PORT))
                    t = time()
                    dataincache = 0
                    print('recv RREP for me',dthash)
                    for datai in datacache:
                        #print('data hash,',dthash,datai[0])
                        if dthash == datai[0]:
                            print('data already in cache',dthash)
                            dataincache = 1
                            break
                    if dataincache == 0:
                        datacache.append((dthash,0))
                        #datacache.append(dthash)
                        #print('data add in cache',dthash)
                        print>>fl,'RREP:time:'+str(t)+',message:'+str(dthash)
                        
                        if mutex.acquire(1):
                            recieved = True                            
                            mutex.release()
                        for i in interest:
                            #print('look up Interest',i[1],i[0])
                            if dthash == i[0]:
                                nextip = i[1]
                                data = [mk,cip,sip,nextip,dthash]
                                data = codedata(data)
                                print('forward RREP to',nextip)			    
                                udpSerSock.sendto(data,(nextip,PORT))
                                sendpktnum += 1
                                break
                    #if nextip != ip:
                    #    data = [mk,cip,sip,nextip,dthash]
                    #    data = codedata(data)
                    #    for neibh in nei:
                    #        udpSerSock.sendto(data,(neibh,PORT))
                    #else:
                    #    for neibh in nei:
                    #        udpSerSock.sendto(data,(neibh,PORT))
                else:
                    pass
            else:
                print("Wrong Type")
        else:
            #time.sleep(20)
	    print('ctl pkt num:',ip,sendpktnum)
            for neibh in nei:
                udpSerSock.sendto(data,(neibh,PORT))
            break
    udpSerSock.close()
    
#global recieved 
#recieved = False
#mutex = Lock()

def main():
    global mutex 
    mutex = Lock()
    ts = Thread(target = send)
    tl = Thread(target = listen)
    ts.start()
    tl.start()

main()
