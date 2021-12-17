# -*- coding: utf-8 -*-
"""
Created on Thu Dec  9 20:10:44 2021

@author: Nolan Adams
"""

#Char to ASCII

def char2ascii(input_str: str):
    ascii_list = []
    for i in input_str:
        ascii_list.append(ord(i))
        print(ord(i))
        
char2ascii("hello, world!")