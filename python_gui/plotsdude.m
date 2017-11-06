clc; clear all; close all;

t = csvread('t.csv');
V = csvread('V.csv');

hold on

plot(t,V,'.')
plot(t,V)
