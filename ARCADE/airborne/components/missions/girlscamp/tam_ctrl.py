# -*- coding: cp1250 -*-
from math import sqrt,atan,tan,sin,cos,atan2
from gradient import *
import random

class Controller:
    """ add controller implementation here  """

    def __init__(self):
        self.gradX = 1
        self.gradY = 1
        self.Strength = {}
        self.nMeasurements={}

        self.x = 0
        self.y = 0
        self.t = 0
        self.gradient_1 = (0,0)
        self.gradient_2 = (0,0)
        self.gradient_old = (0,0)

        self.x_tar = 0
        self.y_tar = 0
    
        self.max_s = -91
        self.max_x = 0
        self.max_y = 0

        self.s_prev1 = -91
        self.s_prev2 = -91
        self.s_curr = -91

        self.fi = 3.14/2

        self.countLocal = 0
        self.countGlobal = 0

        self.globalSearch = 1

        #controller parameters
        self.K = 2
        self.K_ran = 1.8
        self.d_fi = 3.14/4
        self.r = 1.2
        self.d_pos = 1.3

        self.initialSearch = [(-2,2),(-2,-2),(2,-2),(2,3),(-3,3),(-3,-3),(3,-3),(3,4),(-4,4),(-4,-4),(4,-4),(4,5)]


        self.x_new = 0
        self.y_new = 0

        self.gx = 0
        self.gy = 0
        
    def decide(self,x,y,s,timeStamp):

        self.max_s = -100

        for key1 in self.Strength:
            for key2 in self.Strength[key1]:
                if self.Strength[key1][key2] > self.max_s:
                    self.max_s = self.Strength[key1][key2]
                    self.max_x = int(key1)
                    self.max_y = int(key2)


        print 'max', self.max_s
        
        self.s_curr = s
        self.updateStrength(x,y,s,timeStamp)        
        self.updatePosition(x,y,s,timeStamp)
        
        
        self.gradient_1 = getGradient(x,y,self.gradX,self.gradY,self.Strength,1)
        self.gradient_2 = getGradient(x,y,self.gradX,self.gradY,self.Strength,2)

        #print 'global search', self.globalSearch
        #print 'countLocal', self.countLocal
        #print 'countGlobal',self.countGlobal

        tag = 0

        if (self.s_curr > -75):

            self.globalSearch = 0

    
        if (self.globalSearch == 0):
            
            (x, y, gx,gy) = self.getNextPosition()

            #self.x_new = (self.x_new + x)/2
            #self.y_new = (self.y_new + y)/2

            #x_new = self.x_new
            #y_new = self.y_new

            x_new =x
            y_new = y
            
            #self.gx = self.gx + gx
            #self.gy = self.gy + gy
            
            #print 'Prvi gradient', self.gradient_1
            print 's_curr',self.s_curr
            #print 's_prev1',self.s_prev1
            #print 's_prev2',self.s_prev2
            
            if (self.countLocal == 0):
                self.x_tar = x_new
                self.y_tar = y_new
                
                (a,b) = scale(gx, gy)
                self.x_tar = self.x + self.K*a
                self.y_tar = self.y + self.K*b

                #print 'poslije',(self.x_tar,self.y_tar)
                print 'dPos',(self.K*a,self.K*b)
                self.gradient_old = (a,b)
                
                tag = 1
        

        if (self.globalSearch == 1) and (self.countGlobal == 0):
                (x_new,y_new) = self.initialSearch.pop(0)
                self.x_tar = x_new
                self.y_tar = y_new
                print 'dPos',(x-x_new,y-y_new)
                tag = 1

        
        self.s_prev2 = self.s_prev1
        self.s_prev1 = s

        
        self.countLocal +=1
        self.countGlobal += 1
        self.countLocal = (self.countLocal % 3)
        self.countGlobal = (self.countGlobal % 6)
        
        return self.x_tar,self.y_tar,tag


    def updateStrength(self,x,y,s,timeStamp):

        dummy={}
        x = int(round(x))
        y = int(round(y))

        if True:
            if self.Strength.has_key(x):

                suma = s
                if self.Strength[x].has_key(y):
                
                    self.Strength[x][y]=self.Strength[x][y]*self.nMeasurements[x][y] + float(s)
                    self.nMeasurements[x][y]+=1

                    dummy = self.Strength[x]
                    dummy[y] = self.Strength[x][y]/self.nMeasurements[x][y]
                    self.Strength[x] = dummy
            
                else:
                    dummy=self.nMeasurements[x]
                    dummy[y] = 1
                    self.nMeasurements[x] = dummy

                    dummy = self.Strength[x]
                    dummy[y] = s
                    self.Strength[x] = dummy
                

            
            else:
                self.Strength[x] = {y:s}
                self.nMeasurements[x] ={y:1}

        #print 'Measurements matrix', self.nMeasurements
        #print 'Strength matrix', self.Strength
        
    def updatePosition(self,x,y,s,timeStamp):
        self.x = x
        self.y = y
        self.t = timeStamp
        


    def getNextPosition(self):
    
        #print 'Strength matrix', self.Strength

        K = self.K
       
        K_ran = self.K_ran

        gx = self.gradient_1[0]
        gy = self.gradient_1[1]

        self.gradient_2 = (0,0)

        K_d = 1
        if ((self.gradient_1[0] == 0.0) and (self.gradient_2[0] == 0.0)) or (self.s_curr == self.s_prev1):

            #Go to maximum
            gx = K_ran * ((self.max_x - self.x)+ K_d*random.random())
  
            #print 'udaljenost od max', (self.max_x - self.x)
              

        if ((self.gradient_1[1] == 0.0) and (self.gradient_2[1] == 0.0)) or (self.s_curr == self.s_prev1):

            gy = K_ran * ((self.max_y - self.y) + K_d*random.random())


        #print 'signal', self.s_curr
        
        if self.s_curr < -65:
            if  (self.s_curr > self.s_prev1) and (self.s_prev1 > self.s_prev2):
                #continue in the same direction
                x = self.x + K * self.gradient_old[0]
                y = self.y + K * self.gradient_old[1]
                (gx,gy) = (self.gradient_old[0], self.gradient_old[1])
                #print 'Same direction - gradient',self.gradient_old[0],self.gradient_old[1]

            else:
                #go in the direction of gradient

                (gx,gy) = scale(gx,gy)                
                x = self.x + K * gx
                y = self.y + K * gy
                self.gradient_old = (gx,gy)
                #self.gradient_1=(gx,gy)
                #print 'Change direction - gradient',gx,gy

        else:
            x = self.x
            y = self.y
            self.gradient_old = (0,0)
            #print 'Mislim da sam u maksimumu',0,0

        gx = K_ran * ((self.max_x - self.x)+ K_d*random.random())
        gy = K_ran * ((self.max_y - self.y) + K_d*random.random())
        x = self.x + K * gx
        y = self.y + K * gy
        
        return x,y,gx,gy

    def spiralSearch(self,x,y):

        self.fi = self.fi + self.d_fi

        #fi bi se trebao namotavati

        x = cos(self.fi)*pow(self.r,self.fi)
        y = sin(self.fi)*pow(self.r,self.fi)

        #print 'fi,x,y',self.fi,x,y
        return x,y

        
def sign(a):

    if a>0:
        return 1
    elif a<0:
        return -1
    else:
        return 0

def scale(x,y):
    if ((abs(x) + abs(y)) > 1.3) or ((abs(x) + abs(y)) < 0.):
        if abs(x) > abs(y):
            #scale to x
            y = sign(y)*abs(y/x)
            x = sign(x)

        else:
            #scale to y
            x = sign(x)*abs(x/y)
            y = sign(y)

    return (x,y)

