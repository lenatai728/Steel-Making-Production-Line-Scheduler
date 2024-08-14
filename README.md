# COMP2432 Group Project - Steel-Making Production Line Scheduler (PLS) by Project Group 52

## Description 
- Year: 2
- Semester: 2
- Subject/Course: COMP2432 Operating Systems
- Project Period: April, 2024

## Programming Language 
- C 

## Content
- PLS_G52.c (source code)

## Scenario / Assumption
Recently, a factory manager found that there were a number of orders which could not be completed on schedule and caused the decline of the company’s profit. After an investigation, it was found that the three plants were not fully utilized because there was no good planning of the production schedule for the three plants, causing the utilization of the plants to become low.

## Steel-Making Production Line Scheduler (PLS)
In this project, We are to create an application that helps the steel-making manufacturer to schedule their production in order to produce the best utilization of the three plants (X, Y, Z) that produce 300, 400 and 500 products per day respectively. 

## Gain
- applied the theory of process scheduling which is learned from COMP2432 to a daily-life scenario and to produce a Schedule for each scheduling algorithm implemented.
- applied the concepts below through implementing fork() and pipe() in Scheduling Module:
    - process creation, management and control 
    - inter-process communication with unnamed pipe 

## Architecture
- Input Module
- Scheduling Module 
- Output Module

## Features
- Command-Line User Interface (Input Module)
    - to allow users to provide info about the orders
    - to perform the functionalities as described below:
- Generating schedules through implementing 3 different scheduling algorithms (Scheduling Module)
- Formatted output of the Schedules (Output Module)
- Formatted output of the Analysis Report comparing the algorithms (Output Module)

## Scheduling Algorithms
- First-Come-First-Served Scheduling (FCFS)
- Priority Scheduling Scheduling (PR)
- Shortest Job First Scheduling (SJF)
- Least Laxity First (LLF)

#### Note that:
- FCFS, PR, SJF algorithms are learned during the course
- LLF algorithm is learned by self-studying

## Least Laxity First Scheduing Algorithm (LLF)
- Improved form of Priority Scheduling
- Laxity = no. of days until order deadline - remaining quantity of order
- The smaller laxity, the higher priority
- Pros: dynamic priority, improved deadline adherence & resource utilization

## Group Members (Group 52)
- Sze Yeung LAM
- Hei Chun LAI
- Xu SUN
- Chin Wai WONG

###### @The Hong Kong Polytechnic University (PolyU)


