# 当检测到人体活动时，会持续高电平2～4秒

import RPi.GPIO as GPIO
#import time

def body_detection():
    HC_SR501 = 20

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(HC_SR501,GPIO.IN)

    try:
        while True:
            if(GPIO.input(HC_SR501) == True):
                #print(time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))+" 警告!附近一米内有人 ")
                return 1;
            else:
                #print(time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))+" 无情况，周围没人! ")
                return 0;
            #time.sleep(1)
                
    except:
        pass

    GPIO.cleanup()
