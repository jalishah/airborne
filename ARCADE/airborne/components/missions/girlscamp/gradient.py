
from math import sqrt
def gradN(x,y,gradX,gradY,strength,d):
    #print strength
    #print 'N'
    f0 = float(strength[x][y])
    gx = 0
    gy = 0
    #print strength[x].has_key(y + d*gradY)
    if strength[x].has_key(y + d*gradY):
        fy = strength[x][y + d*gradY]
        gy = (fy - f0)/(d*gradY)


    return gx,gy

def gradE(x,y,gradX,gradY,strength,d):
    #print strength
    #print 'E'
    f0 = float(strength[x][y])
    gx = 0
    gy = 0
    #print strength.has_key(x + d*gradX)
    if strength.has_key(x + d*gradX):
        if strength[x + d*gradX].has_key(y):
            fx = strength[x + d*gradX][y]
            gx = (fx - f0)/(d*gradY)
            

    return gx,gy

def gradS(x,y,gradX,gradY,strength,d):
    #print strength
    #print 'S'
    f0 = float(strength[x][y])
    gx = 0
    gy = 0

    #print strength[x].has_key(y - d*gradY)
    
    if strength[x].has_key(y - d*gradY):
        fy = strength[x][y - d*gradY]
        gy = (f0 - fy)/(d*gradY)
        

    return gx,gy

def gradW(x,y,gradX,gradY,strength,d):
    #print strength
    #print 'W'
    #print x - d*gradX,y
    f0 = float(strength[x][y])
    gx = 0
    gy = 0
    #print strength.has_key(x - d*gradX)
    if strength.has_key(x - d*gradX):
        if strength[x - d*gradX].has_key(y):
            fx = strength[x - d*gradX][y]
            gx = (f0 - fx)/(d*gradY)
            

    return gx,gy

def gradNE(x,y,gradX,gradY,strength,d):
    #print strength
    #print 'NE'
    f0 = float(strength[x][y])
    gx = 0
    gy = 0
    #print strength.has_key(x + d*gradX)
    if strength.has_key(x + d*gradX):
        if strength[x + d*gradX].has_key(y + d*gradY):
            f = strength[x + d*gradX][y + d*gradY]
            gx = (f - f0)/(sqrt(2)*d*gradX)
            gy = (f - f0)/(sqrt(2)*d*gradY)
            

    return gx,gy


def gradNW(x,y,gradX,gradY,strength,d):
    #print strength
    #print 'NW'
    f0 = float(strength[x][y])
    gx = 0
    gy = 0
    #print strength.has_key(x - d*gradX)
    if strength.has_key(x - d*gradX):
        if strength[x - d*gradX].has_key(y + d*gradY):
            f = strength[x - d*gradX][y + d*gradY]
            gx = (f - f0)/(sqrt(2)*d*gradX)
            gy = -1*(f - f0)/(sqrt(2)*d*gradY)
            

    return gx,gy

def gradSW(x,y,gradX,gradY,strength,d):
    #print strength
    #print 'SW'
    f0 = float(strength[x][y])
    gx = 0
    gy = 0
    #print strength.has_key(x - d*gradX)
    if strength.has_key(x - d*gradX):
        if strength[x - d*gradX].has_key(y - d*gradY):
            f = strength[x - d*gradX][y - d*gradY]
            gx = -1*(f - f0)/(sqrt(2)*d*gradX)
            gy = -1*(f - f0)/(sqrt(2)*d*gradY)
            

    return gx,gy

def gradSE(x,y,gradX,gradY,strength,d):
    #print strength
    #print 'SE'
    f0 = float(strength[x][y])
    gx = 0
    gy = 0
    #print strength.has_key(x + d*gradX)
    if strength.has_key(x + d*gradX):
        if strength[x + d*gradX].has_key(y - d*gradY):
            f = strength[x + d*gradX][y - d*gradY]
            gx = (f - f0)/(sqrt(2)*d*gradX)
            gy = -1*(f - f0)/(sqrt(2)*d*gradY)
            

    return gx,gy

def getGradient(x,y,gradX,gradY,strength,d):
    
    #print 'STRENGTH', strength
    #print strength.keys()
        
    x = int(round(x))
    y = int(round(y))
    #print 'Trazim gradijent na',(x,y),'d ',d,'gradX',gradX,'gradY',gradY

        
    (x1,y1) = gradN(x,y,gradX,gradY,strength,d)
    (x2,y2) = gradS(x,y,gradX,gradY,strength,d)
    (x3,y3) = gradW(x,y,gradX,gradY,strength,d)
    (x4,y4) = gradE(x,y,gradX,gradY,strength,d)
    (x5,y5) = gradNW(x,y,gradX,gradY,strength,d)
    (x6,y6) = gradNE(x,y,gradX,gradY,strength,d)
    (x7,y7) = gradSW(x,y,gradX,gradY,strength,d)
    (x8,y8) = gradSE(x,y,gradX,gradY,strength,d)

    fx = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8
    fy = y1 + y2 + y3 + y4 + y5 + y6 + y7 + y8

    return fx,fy

        
