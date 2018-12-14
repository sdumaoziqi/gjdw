#!/usr/bin/env python3
import argparse
import json
import numpy
import os
import random
import re
import subprocess
import sys
import time
import threading

args = None
logFile = None
randomFile = None



producerAccounts = [
    'useraaaaaaaa',
    'useraaaaaaab',
    'useraaaaaaac',
]

def jsonArg(a):
    return " '" + json.dumps(a) + "' "

def run(args):
    #print('get_random.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('get_random.py: exiting because of error')
        sys.exit(1)

def background(args):
    print('get_random.py:', args)
    logFile.write(args + '\n')
    return subprocess.Popen(args, shell=True)

def getOutput(args):
    print('get_random.py:', args)
    logFile.write(args + '\n')
    proc = subprocess.Popen(args, shell=True, stdout=subprocess.PIPE)
    return proc.communicate()[0].decode('utf-8')

def getJsonOutput(args):
    return json.loads(getOutput(args))

def sleep(t):
    print('sleep', t, '...')
    time.sleep(t)
    print('resume')

def setContract():
    run('g++ -o gen_hash gen_hash.cpp')
    for producer in producerAccounts:
        run(args.cleos + 'system buyram %s %s "1000.0000 GOC"' %(producer, producer))  #买一些内存
    sleep(1)
    run(args.eosiocpp + '-o random.wast random.cpp')
    sleep(1)
    run(args.eosiocpp + '-g random.abi random.cpp')
    sleep(1)
    run(args.cleos + 'system newaccount --transfer useraaaaaaaa useruseruser GOC8ZjbDEi872aLpuuAjnd76NYW6KzPaf6RBSuwXcHmKm7A1sxayV --stake-net "200.0000 GOC" --stake-cpu "200.0000 GOC" --buy-ram "200.0000 GOC" ')
    run(args.cleos + 'set contract useruseruser ./../random/ ')
    run(args.cleos + 'get table useruseruser 1 randoms')


def pushHash(producer, term, hash):
    run(args.cleos + 'push action useruseruser pushhash \'["%s", %d, "%s"]\' -p %s' % (producer, term, hash, producer))

def pushValue(producer, term, value):
    run(args.cleos + 'push action useruseruser pushvalue \'["%s", %d, "%s"]\' -p %s' % (producer, term, value, producer))

def runGetOutput(args):
    logFile.write(args + '\n')
    res = subprocess.getoutput(args)
    return res

def getHash(value):
    res = runGetOutput("./gen_hash %s" % (value))
    return res

def getRandom():
    return numpy.random.randint(100000000)


def getTable(term):
    res = runGetOutput(args.cleos + 'get table useruseruser %d randoms' %(term))
    j = json.loads(res)
    value_count = j['rows'][0]['value_count']
    hash_count = j['rows'][0]['hash_count']
    if hash_count > 0 and hash_count == value_count:
        value = ""
        i = 0
        while i < j['rows'][0]['hash_count']:
            value += j['rows'][0]['producers_hash'][i]['value']
            i += 1
        hash = getHash(value)
        randomFile.write('%s  term #%04d  %25s \n' %(time.strftime('%Y-%m-%d %H:%M:%S',time.localtime()), term, hash))
        print('term #', term, ' hash:', hash)
    

class calRandom(threading.Thread):
    def __init__(_self, threadID, name):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
    def run(self):
        sleep(15)
        print('开始计算Random')



# Command Line Arguments

parser = argparse.ArgumentParser()


parser.add_argument('--eosiocpp', metavar='', help="Eosiocpp command", default='../../build/tools/eosiocpp ')
parser.add_argument('--cleos', metavar='', help="Cleos command", default='../../build/programs/cleos/cleos --wallet-url http://127.0.0.1:6666 ')
parser.add_argument('--nodeos', metavar='', help="Path to nodeos binary", default='../../build/programs/nodeos/nodeos')
parser.add_argument('--keosd', metavar='', help="Path to keosd binary", default='../../build/programs/keosd/keosd')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory", default='../../build/contracts/')
parser.add_argument('--nodes-dir', metavar='', help="Path to nodes directory", default='./nodes/')
parser.add_argument('--wallet-dir', metavar='', help="Path to wallet directory", default='./wallet/')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./gen_random_log.log')
parser.add_argument('--random-path', metavar='', help="Path to random file", default='./random.txt')
parser.add_argument('-r', '--round', metavar='', help="Num of random", default=5)

parser.add_argument('-a', '--all', action='store_true', help="Do everything marked with (*)")
parser.add_argument('-H', '--http-port', type=int, default=8000, metavar='', help='HTTP port for cleos')


        
args = parser.parse_args()

args.cleos += '--url http://127.0.0.1:%d ' % args.http_port

logFile = open(args.log_path, 'a')
randomFile = open(args.random_path, 'a')


logFile.write('\n\n' + '*' * 80 + '\n\n')
randomFile.write('\n' + '*' * 80 + '\n')



term = 0


dict = {'useraaaaaaaa': "1", 'useraaaaaaab': "2", 'useraaaaaaac': "3"}

setContract()




sleep(2)

while term < int(args.round):
    print('*' * 60)
    for producer in producerAccounts:
        random = getRandom()
        dict[producer] = random
        hash = getHash(random)
        pushHash(producer, term, hash)
    sleep(1)
    for producer in producerAccounts:
        pushValue(producer, term, str(dict[producer]))
    sleep(1)
    getTable(term)
    term+=1
    sleep(1)


