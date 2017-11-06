clc; clear all; close all;

t = csvread('t_output1.csv');
V = csvread('V_output2.csv');

hold on

plot(t,V,'.')
plot(t,V)
