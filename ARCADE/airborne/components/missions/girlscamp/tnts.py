def enum(**enums):
    return type('Enum', (), enums)

Direction = enum(UP = 'up', DOWN = 'down', RIGHT = 'right', LEFT = 'left')
Gradient = enum(POSITIVE = '+', NEGATIVE = '-')
Mode = enum(SEARCH = 'search', FOLLOW = 'follow')
ChangePosition = enum(CHANGE = 1, DONT_CHANGE = 0)
MoveConstant = 1
CounterConstant = 3

class VisitedLocation :
    def __init__ (self, X, Y, S, directionTo):
        self.position = Position(X,Y)
        self.strength = S
        self.direction = directionTo

    def updateValues(self, X, Y, S, directionTo):
        self.position.x = X
        self.position.y = Y
        self.strength = S
        self.direction = directionTo

    def setDirection (self, newDirection):
        self.direction = newDirection

class Position:
    def __init__ (self, X,Y):
        self.x = X
        self.y = Y
        
class Controller:
    def __init__ (self, X, Y, S):
        self.x = X
        self.y = Y
        self.s = S
        self.mode = Mode.SEARCH
        self.currentDirection = Direction.UP
        self.newX = 0
        self.newY = 0
        self.treshold = 2

        self.destination = Position(0,0)

        A = Position(-2.5, 7.5)
        B = Position(2.5, 7.5)
        C = Position(2.5, -7.5)
        D = Position(-2.5, -7.5)

        self.neighbourNodeList = [A,B,C,D,A]
        self.directionsList = [Direction.UP, Direction.RIGHT, Direction.DOWN, Direction.LEFT, Direction.UP]
        self.visitedLocations = []
        self.lastStrengthValues = []
        self.changeDestination = ChangePosition.CHANGE
        self.updateCounter = 0

    def moveUp (self):
        self.newY = self.newY + MoveConstant
        print 'move up \n'

    def moveDown (self):
        self.newY = self.newY - MoveConstant
        print 'move down \n'

    def moveRight (self):
        self.newX = self.newX + MoveConstant
        print 'move right \n'

    def moveLeft (self):
        self.newX = self.newX - MoveConstant
        print 'move left \n'

    def update (self, X, Y, S):
        self.x = X
        self.y = Y
        self.s = S
        self.newX = X
        self.newY = Y

    def continueInSameDirection(self, direction):
        if direction == Direction.UP:
            self.moveUp()
        elif direction == Direction.DOWN:
            self.moveDown()
        elif direction == Direction.RIGHT:
            self.moveRight()
        elif direction == Direction.LEFT:
            self.moveLeft()

    def decisionForPositiveGradient(self):
        self.continueInSameDirection(self.visitedLocations[1].direction)
        newDirection = self.visitedLocations[1].direction
        tempLocation = VisitedLocation(self.x, self.y, self.s, newDirection)
        self.visitedLocations.append(tempLocation)
        self.visitedLocations.pop(0)

    def opositeDirection (self, direction):
        if direction == Direction.UP:
            return Direction.DOWN
        elif direction == Direction.DOWN:
            return Direction.UP
        elif direction == Direction.RIGHT:
            return Direction.LEFT
        elif direction == Direction.LEFT:
            return Direction.RIGHT

    def crossingDirection (self, direction):
        if direction == Direction.UP:
            self.moveDown()
            self.moveRight()
            return Direction.RIGHT
        elif direction == Direction.DOWN:
            self.moveUp()
            self.moveLeft()
            return Direction.LEFT
        elif direction == Direction.RIGHT:
            self.moveLeft()
            self.moveUp()
            return Direction.UP
        elif direction == Direction.LEFT:
            self.moveRight()
            self.moveDown()
            return Direction.DOWN

    def calculateNewGradient (self, location):
        if self.s < location.strength:
            return Gradient.NEGATIVE

        return Gradient.POSITIVE

    def generateNextSearchPosition(self):
        position = Position(0,0)
        currentPos = Position(self.x, self.y)
        tag = ChangePosition.DONT_CHANGE

        if currentPos.x == 0 and currentPos.y == 0:
            position = self.neighbourNodeList[0]
        else:
            position, self.direction = self.findNextNode(currentPos)

        if position:
            self.newX = position.x
            self.newY = position.y
            tag = ChangePosition.CHANGE
            
        return tag

    def generateNextFollowPosition(self):
        if len(self.visitedLocations) == 0:
            self.visitedLocations.append(VisitedLocation(0, 0, 0, self.currentDirection))            
            self.visitedLocations.append(VisitedLocation(self.x, self.y, self.s, self.currentDirection))
            self.continueInSameDirection(self.currentDirection)
        else:
            if self.visitedLocations[0].direction == self.visitedLocations[1].direction:
                gradient = self.calculateNewGradient(self.visitedLocations[1])
                if gradient == Gradient.POSITIVE:
                    self.decisionForPositiveGradient()
                else:
                    newDirection = self.crossingDirection(self.visitedLocations[1].direction)
                    #print 'crossing', self.newX, self.newY
                    self.visitedLocations[1].setDirection(newDirection)
            elif self.visitedLocations[0].direction != self.visitedLocations[1].direction and self.visitedLocations[0].direction != self.opositeDirection(self.visitedLocations[1].direction):
                gradient = self.calculateNewGradient(self.visitedLocations[1])
                if gradient == Gradient.POSITIVE:
                    self.decisionForPositiveGradient()
                else:
                    newDirection = self.opositeDirection(self.visitedLocations[1].direction)
                    self.visitedLocations[1].setDirection(newDirection)
                    self.continueInSameDirection(newDirection)
                    self.continueInSameDirection(newDirection)
                    
            
                    
            

        #print 'newX', self.newX, 'newY', self.newY
        self.destination.x = self.newX
        self.destination.y = self.newY
        
    
    def decide (self, X, Y, S, timeStamp):

        self.updateCounter += 1
        if self.updateCounter < CounterConstant:
            return 0,0, ChangePosition.DONT_CHANGE

        self.updateCounter = 0
        
        self.update(X, Y, S)
        self.lastStrengthValues.append(S)

        if self.s == -65:
            print 'max found ', self.s
            self.destination.x = self.x
            self.destination.y = self.y
            return self.destination.x, self.destination.y, ChangePosition.CHANGE
        
        if len(self.lastStrengthValues) > 4:
            self.lastStrengthValues.pop(0)
            if self.changeDestination == ChangePosition.CHANGE:
                if self.lastStrengthValues[0] == self.lastStrengthValues[2] and self.lastStrengthValues[1] == self.lastStrengthValues[3]:
                    self.changeDestination = ChangePosition.DONT_CHANGE
                    print 'im stuck'
                    self.getOutOfHere()
                    self.destination.x = self.newX
                    self.destination.y = self.newY
                    return self.destination.x, self.destination.y, ChangePosition.CHANGE
            if self.changeDestination == ChangePosition.DONT_CHANGE:
                closeX = self.x - self.destination.x
                closeY = self.y - self.destination.y
                if (closeX < self.treshold and closeX > -self.treshold) and (closeY < self.treshold and closeY > -self.treshold):
                    for i in range(0,len(self.lastStrengthValues)):
                        self.lastStrengthValues.pop(0)
                    for i in range(0,len(self.visitedLocations)):
                        self.visitedLocations.pop(0)            
                                
                    self.changeDestination = ChangePosition.CHANGE
                    print 'im free'
                    print 'current ', self.x, ' ', self.y
                    print 'should be ', self.destination.x, ' ', self.destination.y
                    print 'close ', closeX, ' ', closeY
                else:
                    print 'im out'
                    return self.destination.x, self.destination.y, ChangePosition.CHANGE
                
                

        print X, Y, S
        
        if self.s >= -91 and self.mode == Mode.SEARCH:
            self.mode = Mode.FOLLOW

        if self.mode == Mode.SEARCH:
            self.generateNextSearchPosition()
                        
        else:
            self.generateNextFollowPosition()

        print 'sent values ', self.destination.x, self.destination.y, ChangePosition.CHANGE
             
        return self.destination.x, self.destination.y, ChangePosition.CHANGE
        

    def findNextNode(self, currentNode):
        for i in range(0,len(self.neighbourNodeList)):
            if currentNode == self.neighbourNodeList[i]:
                self.destination.x = self.neighbourNodeList[i+1].x
                self.destination.y = self.neighbourNodeList[i+1].y                
                return self.neighbourNodeList[i+1], self.directionList[i+1]
        return None, self.currentDirection

    def getOutOfHere(self):
        print 'get out of here \n'
        if self.s < -66:           
            direction = self.currentDirection
            newDirection = self.crossingDirection(direction)
            newDirection = self.crossingDirection(direction)
            

        
            
            
                     
        
        
    

#controller = Controller(0,0,null)
#visitedLocations = []

        
