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

args = None
logFile = None


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
    run(args.cleos + 'system newaccount --transfer useraaaaaaaa zmaozmaozmao GOC8ZjbDEi872aLpuuAjnd76NYW6KzPaf6RBSuwXcHmKm7A1sxayV --stake-net "200.0000 GOC" --stake-cpu "200.0000 GOC" --buy-ram "200.0000 GOC" ')
    run(args.cleos + 'set contract zmaozmaozmao ./../random/ ')
    run(args.cleos + 'get table zmaozmaozmao 1 randoms')


def pushHash(producer, term, hash):
    run(args.cleos + 'push action zmaozmaozmao pushhash \'["%s", %d, "%s"]\' -p %s' % (producer, term, hash, producer))

def pushValue(producer, term, value):
    run(args.cleos + 'push action zmaozmaozmao pushvalue \'["%s", %d, "%s"]\' -p %s' % (producer, term, value, producer))

def runGetOutput(args):
    logFile.write(args + '\n')
    res = subprocess.getoutput(args)
    return res

def getHash(value):
    res = runGetOutput("./gen_hash %d" % (value))
    return res

def getRandom():
    return numpy.random.randint(100000000)


# Command Line Arguments

parser = argparse.ArgumentParser()



parser.add_argument('--cleos', metavar='', help="Cleos command", default='../../build/programs/cleos/cleos --wallet-url http://127.0.0.1:6666 ')
parser.add_argument('--nodeos', metavar='', help="Path to nodeos binary", default='../../build/programs/nodeos/nodeos')
parser.add_argument('--keosd', metavar='', help="Path to keosd binary", default='../../build/programs/keosd/keosd')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory", default='../../build/contracts/')
parser.add_argument('--nodes-dir', metavar='', help="Path to nodes directory", default='./nodes/')
parser.add_argument('--wallet-dir', metavar='', help="Path to wallet directory", default='./wallet/')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./gen_random_log.log')
parser.add_argument('-a', '--all', action='store_true', help="Do everything marked with (*)")
parser.add_argument('-H', '--http-port', type=int, default=8000, metavar='', help='HTTP port for cleos')


        
args = parser.parse_args()

args.cleos += '--url http://127.0.0.1:%d ' % args.http_port

logFile = open(args.log_path, 'a')

logFile.write('\n\n' + '*' * 80 + '\n\n\n')


term = 1


dict = {'useraaaaaaaa': 1, 'useraaaaaaab': 2, 'useraaaaaaac': 3}

setContract()

while term < 10:
    print('*' * 60)
    sleep(2)
    for producer in producerAccounts:
        random = getRandom()
        dict[producer] = random
        hash = getHash(random)
        pushHash(producer, term, hash)
        sleep(1)
    for producer in producerAccounts:
        pushValue(producer, term, str(dict[producer]))
        sleep(1)
    term+=1


