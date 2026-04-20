#!/usr/bin/env sh


nasm -f elf64 heapsort_knuth.asm -o heapsort_knuth.o
cc -no-pie -o test_heapsort main.c heapsort_knuth.o
./test_heapsort
