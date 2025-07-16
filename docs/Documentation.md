# **Documentation**

## **Function Headers**

### **Object**

---

- #### Contructors

    - [`Object Object(String objName, String modelName, String textureName, String shaderName)`](#object-objectstring-objname-string-modelname-string-texturename-string-shadername)
    - [`Object Object(String objName, String modelName, String textureName, String shaderName, string scriptName)`](#object-objectstring-objname-string-modelname-string-texturename-string-shadername-string-scriptname)

- #### Destructors

    - [`void destroy()`](#void-destroy)

- #### Getters

    - [`Number[3] getPosition()`](#number3-getposition)
    - [`Number[3] getRotation()`](#number3-getrotation)
    - [`Number[3] getScale()`](#number3-getscale)
    - [`String getName()`](#string-getname)

- #### Setters

    - [`void setPosition(Number x, Number y, Number z)`](#void-setpositionnumber-x-number-y-number-z)
    - [`void setRotation(Number pitch, Number yaw, Number roll)`](#void-setrotationnumber-pitch-number-yaw-number-roll)
    - [`void setScale(Number x, Number y, Number z)`](#void-setscalenumber-x-number-y-number-z)
    - [`void setScale(Number scale)`](#void-setscalenumber-scale)

- #### Physics

    - [`void move(Number x, Number y, Number z)`](#void-movenumber-x-number-y-number-z)
    - [`void rotate(Number pitch, Number yaw, Number roll)`](#void-rotatenumber-pitch-number-yaw-number-roll)
    - [`Bool checkCollision(String objName)`](#bool-checkcollisionstring-objname)
    - [`Bool checkCollison(Object obj)`](#bool-checkcollisionobject-obj)

- #### Trackers

    - [`void moveToward(String objName, Number speed)`](#void-movetowardstring-objname-number-speed)
    - [`void moveToward(Object obj, Number speed)`](#void-movetowardobject-obj-number-speed)
    - [`void lookAt(String objName)`](#void-lookatstring-objname)
    - [`void lookAt(Object obj)`](#void-lookatobject-obj)

### **Player**

---

- [`Object getPlayer()`](#object-getplayer)
- [`String getPlayerName()`](#string-getplayername)

### **Scene**

---

- [`void createObject(String objName, String modelName, String textureName, String shaderName)`](#void-createobjectstring-objname-string-modelname-string-texturename-string-shadername)
- [`void createObject(String objName, String modelName, String textureName, String shaderName, String scriptName)`](#void-createobjectstring-objname-string-modelname-string-texturename-string-shadername-string-scriptname)
- [`void destroyObject(String objName)`](#void-destroyobjectstring-objname)
- [`void destroyObject(Object obj)`](#void-destroyobjectobject-obj)
- [`Object getObject(String objName)`](#object-getobjectstring-objname)

### **Input**

---

- [`Bool isKeyPressed(String key)`](#bool-iskeypressesstring-key)

## **Function Definitions**

### **Object**

---

**Contructors**

---

#### `Object Object(String objName, String modelName, String textureName, String shaderName)`

Creates and returns a new Object with a specified name, model, texture, and shader.

*Parameters:*
- `String` objName - name of new object
- `String` modelName - name of mesh
- `String` textureName - name of texture
- `String` shaderName - name of shader

*Returns:* An `Object` instance.

*Example:*

    local obj = Object("crate", "cube", "wood.png", "default")

#### `Object Object(String objName, String modelName, String textureName, String shaderName, string scriptName)`

 Creates and returns a new Object with a specified name, model, texture, shader, and script.

*Parameters:*
- `String` objName
- `String` modelName
- `String` textureName
- `String` shaderName
- `String` scriptName

*Returns:*

- `Object` obj

*Example:*

    local obj = Object("enemy", "pyramid", "red.png", "default", "follow")

**Destructors**

---

#### `void destroy()`

Destroys `this`.

*Example:*

    obj:destroy()

**Getters**

---

#### `Number[3] getPosition()`

Returns the x, y, and z positions of `this`.

*Returns:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    local x, y, z = obj:getPosition()

#### `Number[3] getRotation()`

Returns the degrees of pitch, yaw, and roll of `this`.

*Returns:*

- `Number` pitch 
- `Number` yaw
- `Number` roll

*Example:*

    local pitch, yaw, roll = obj:getRotation()

#### `Number[3] getScale()`

Returns the x, y, and z scales of `this`.

*Returns:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    local x, y, z = obj:getScale()

#### `String getName()`

Returns the name of `this`.

*Returns:*

- `String` name

*Example:*

    local objName = obj:getName()

**Setters**

---

#### `void setPosition(Number x, Number y, Number z)`

Sets the x, y, and z positions of `this` to specified values.

*Parameters:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    obj:setPosition(1, 2, 3)

#### `void setRotation(Number pitch, Number yaw, Number roll)`

Sets the pitch, yaw, and roll of `this` to specified degrees.

*Parameters:*

- `Number` pitch
- `Number` yaw
- `Number` roll

*Example:*

    obj:setRotation(90, 180, 270)

#### `void setScale(Number x, Number y, Number z)`

Sets the x, y, and z scaling of `this` to specified values.

*Parameters:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    obj:setScale(2, 3, 4)

#### `void setScale(Number scale)`

Sets the scaling of `this` to specified value.

*Parameters:*

- `Number` scale

*Example:*

    obj:setScale(2)

**Physics**

---

#### `void move(Number x, Number y, Number z)`

Adds velocity x, y, and z to `this`.

*Parameters:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    obj:move(1, 2, 3)

#### `void rotate(Number pitch, Number yaw, Number roll)`

Adds pitch, yaw, and roll degrees to `this`'s rotation.

*Parameters:*

- `Number` pitch
- `Number` yaw
- `Number` roll

*Example:*

    obj:rotate(90, 180, 270)

#### `Bool checkCollision(String objName)`

Checks if `this` has collided with specific object.

*Parameters:*

- `String` objName

*Returns:*

- `Bool` isCollided

*Example:*

    obj:checkCollision("otherObjName")

#### `Bool checkCollision(Object obj)`

Checks if `this` has collided with specified object.

*Parameters:*

- `Object` obj

*Returns:*

- `Bool` isCollided

*Example:*

    obj:checkCollision(otherObj)

**Trackers**

---

#### `void moveToward(String objName, Number speed)`  

Applies speed to `this`'s velocity in the direction of specified object.

*Parameters:*

- `String` objName
- `Number` speed

*Example:*

    obj:moveToward("otherObjName", 2)

#### `void moveToward(Object obj, Number speed)`  

Applies speed to `this`'s velocity in the direction of specified object.

*Parameters:*

- `Object` obj
- `Number` speed

*Example:*

    obj:moveToward(otherObj, 2)

#### `void lookAt(String objName)`  

Sets `this`'s yaw to the direction of specified object.

*Parameters:*

- `String` objName

*Example:*

    obj:lookAt("otherObjName")

#### `void lookAt(Object obj)`  

Sets `this`'s yaw to the direction of specified object.

*Parameters:*

- `Object` obj

*Example:*

    obj:lookAt(otherObj)

### **Player**

---

#### `Object getPlayer()`  

Returns the player object.

*Returns:*

- `Object` playerObj

*Example:*

    local player = getPlayer()

#### `String getPlayerName()`  

Returns the player object's name.

*Returns:*

- `String` playerObjName

*Example:*

    local playerName = getPlayerName()

### **Scene**

---

#### `void createObject(String objName, String modelName, String textureName, String shaderName)`  

Creates a new Object with a specified name, model, texture, and shader.

*Parameters:*
- `String` objName
- `String` modelName
- `String` textureName
- `String` shaderName

*Example:*

    createObject("crate", "cube", "wood.png", "default")

#### `void createObject(String objName, String modelName, String textureName, String shaderName, String scriptName)`  

Creates a new Object with a specified name, model, texture, shader, and script.

*Parameters:*
- `String` objName
- `String` modelName
- `String` textureName
- `String` shaderName
- `String` scriptName

*Example:*

    createObject("enemy", "pyramid", "red.png", "default", "follow")

#### `void destroyObject(String objName)`  

Destroys the specified object.

*Parameters:*

- `String` objName

*Example:*

    destroyObject("crate")

#### `void destroyObject(Object obj)`  

Destroys the specified object.

*Parameters:*

- `Object` obj

*Example:*

    destroyObject(crateObj)

#### `Object getObject(String objName)`  

Returns the specified object.

*Parameters:*

- `String` objName

*Returns:*

- `Object` obj

*Example:*

    local obj = getObject("enemy")

### **Input**

---

#### `Bool isKeyPressed(String key)`  

Return whether or not the specified key is pressed down.

*Parameters:*

- `String` key

*Returns:*

- `Bool` isPressed

*Example:*

    if (isKeyPressed("W")) then
        obj:move(1, 0, 0)
    end