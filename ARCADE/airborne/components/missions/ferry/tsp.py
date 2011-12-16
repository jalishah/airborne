#!/usr/bin/python

import random
from math import sqrt

def rand_seq(size):
    '''generates values in random order
    equivalent to using shuffle in random,
    without generating all values at once'''
    values=range(size)
    for i in xrange(size):
        # pick a random index into remaining values
        j=i+int(random.random()*(size-i))
        # swap the values
        values[j],values[i]=values[i],values[j]
        # return the swapped value
        yield values[i] 

def all_pairs(size):
    '''generates all i,j pairs for i,j from 0-size'''
    for i in rand_seq(size):
        for j in rand_seq(size):
            yield (i,j)

def reversed_sections(tour):
    '''generator to return all possible variations where the section between two cities are swapped'''
    for i,j in all_pairs(len(tour)):
        if i != j:
            copy=tour[:]
            if i < j:
                copy[i:j+1]=reversed(tour[i:j+1])
            else:
                copy[i+1:]=reversed(tour[:j])
                copy[:j]=reversed(tour[i+1:])
            if copy != tour: # no point returning the same tour
                yield copy

def swapped_cities(tour):
    '''generator to create all possible variations where two cities have been swapped'''
    for i,j in all_pairs(len(tour)):
        if i < j:
            copy=tour[:]
            copy[i],copy[j]=tour[j],tour[i]
            yield copy

def cartesian_matrix(coords):
    '''create a distance matrix for the city coords that uses straight line distance'''
    matrix={}
    for i,(x1,y1) in enumerate(coords):
        for j,(x2,y2) in enumerate(coords):
            dx,dy=x1-x2,y1-y2
            dist=sqrt(dx*dx + dy*dy)
            matrix[i,j]=dist
    return matrix

def tour_length(matrix,tour):
    '''total up the total length of the tour based on the distance matrix'''
    total=0
    num_cities=len(tour)
    for i in range(num_cities):
        j=(i+1)%num_cities
        city_i=tour[i]
        city_j=tour[j]
        total+=matrix[city_i,city_j]
    return total


def init_random_tour(tour_length):
    tour=range(tour_length)
    random.shuffle(tour)
    return tour


def run_hillclimb(init_function,move_operator,objective_function,max_iterations):
    from hillclimb import hillclimb_and_restart
    iterations,score,best=hillclimb_and_restart(init_function,move_operator,objective_function,max_iterations)
    return iterations,score,best


def run_tsp(coords):
    max_iterations = 4000
    move_operator = reversed_sections
    run_algorithm = run_hillclimb
    
    start_temp,alpha=None,None
    # setup the things tsp specific parts hillclimb needs
    init_function=lambda: init_random_tour(len(coords))
    matrix=cartesian_matrix(coords)
    objective_function=lambda tour: -tour_length(matrix,tour)
    iterations,score,best=run_algorithm(init_function,move_operator,objective_function,max_iterations)

    return best
