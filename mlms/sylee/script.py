#%%
from turtle import delay
import matplotlib.pyplot as plt
import os
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element, dump, ElementTree
import matplotlib.pyplot as plt 
import numpy as np
import scipy.stats 
from scipy.stats import norm
import random

count = int(1)

delay_count = []
jitter_count = []

xml_file = '/home/sylee/workspace/ns-allinone-3.36.1/ns-3.36.1/examples/mlms/results.xml'
doc = ET.parse(xml_file)

#root 노드 가져오기
root = doc.getroot()

def set_number():
    return random.randrange(1,11)

def drawing(delay_count, jitter_count):
    global count
    
    if (len(delay_count) == 0 or len(jitter_count) == 0):
        return (-1)
    
    delay_count = list(map(int, delay_count))
    jitter_count = list(map(int, jitter_count))

    delay_mean = np.mean(delay_count) # 평균
    delay_std = np.std(delay_count) # 표준편차

    jitter_mean = np.mean(jitter_count) 
    jitter_std = np.std(jitter_count)

    delay_dist = scipy.stats.norm(delay_mean, delay_std) # 정규분포표
    jitter_dist = scipy.stats.norm(jitter_mean, jitter_std)


    ###### 최대값 1250 알아서 볼 수 있도록,,,
    d = np.linspace(0, 1250, 1000)
    density_d = delay_dist.cdf(d) 
    plt.plot(d, density_d)
    plt.xlabel('Count') # xlabel 타이틀 Count로 변경
    plt.ylabel('')  
    plt.title("delay") 
    # plt.show()
    plt.savefig(str(count)+".jpg")
    count += 1
    plt.close()

    j = np.linspace(0, 70000, 1000)
    density_j = jitter_dist.cdf(j)
    plt.plot(j, density_j)
    plt.xlabel('Count') # xlabel 타이틀 Count로 변경
    plt.ylabel('')
    plt.title("jitter")
    # plt.show()
    plt.savefig(str(count)+".jpg")
    count += 1
    plt.close()


for flow_tag in root.iter("Flow"):
    delay_count.clear()
    jitter_count.clear()
    if (flow_tag.find("delayHistogram")):
        for delayHistogram in flow_tag.iter("delayHistogram"):
            for bin in delayHistogram.iter("bin"):
                delayHistogram.iter("bin")
                delay_count.append(bin.get("count"))
    if (flow_tag.find("jitterHistogram")):
        for jitterHistogram in flow_tag.iter("jitterHistogram"):
            for bin in jitterHistogram.iter("bin"):
                jitterHistogram.iter("bin")
                jitter_count.append(bin.get("count"))
    drawing (delay_count, jitter_count)


with open('/home/sylee/workspace/ns-allinone-3.36.1/ns-3.36.1/examples/mlms/results.xml', 'rt', encoding='UTF-8') as F:
    f = F.read()

# %%
