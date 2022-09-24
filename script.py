#%%
from itertools import count
from turtle import delay
import matplotlib.pyplot as plt
import os
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element, dump, ElementTree
import matplotlib.pyplot as plt 
import numpy as np 
from scipy.stats import norm

delay_count = []
jitter_count = []

xml_file = '/home/sylee/workspace/ns-allinone-3.36.1/ns-3.36.1/examples/mlms/reduce-results.xml'
doc = ET.parse(xml_file)

#root 노드 가져오기
root = doc.getroot()

def drawing(delay, jitter):
    if (len(delay) == 0 or len(jitter) == 0):
        return (-1)
    
    print (delay_count, jitter_count)
    plt.hist(delay_count, alpha=0.7)
    plt.hist(jitter_count, alpha=0.7)
    plt.show()


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
