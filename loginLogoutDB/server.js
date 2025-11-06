    //COPY
require("dotenv").config()
const jwt = require("jsonwebtoken")
const bcrypt = require("bcrypt")
const cookieParser = require('cookie-parser')
const express = require("express")
const db = require("better-sqlite3")("KZOLM.db")
db.pragma("journal_mode = WAL")
    //END 

//database setup 
    //COPY
const createTables = db.transaction(()=>{
 db.prepare(
    `
    CREATE TABLE IF NOT EXISTS users(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username STRING NOT NULL UNIQUE,
    password STRING NOT NULL
    )
    `
).run()
})

createTables()

const app = express()

app.set("view engine","ejs")
app.use(express.urlencoded({extended:false}))
app.use(express.static("public"))
app.use(cookieParser())
    //END

app.use(function(req,res,next){
    res.locals.errors = []

//try to decode incoming cookie
    //not sure
    try{
        const decoded = jwt.verify(req.cookies.userCookie, process.env.JWTSECRET)
        req.user = decoded
    } catch(err){
        req.user = false
    }

    res.locals.user = req.user
    console.log(req.user)

    next()
})
    //END
    
app.get("/",(req,res) => {
    if(req.user){
       return res.render("dashboard")
    }
    res.render("homepage")
})

app.get("/login",(req,res) => {
    res.render("login")
})

//for logout
    //COPY
app.get("/logout",(req,res) => {
    res.clearCookie("userCookie")
    res.redirect("/")
})
    //END

//for login
    //COPY
app.post("/login",(req,res) =>{
    //check username and password validity
    let errors = []

    if(typeof req.body.username !== "string") req.body.username = ""
    if(typeof req.body.password !== "string") req.body.password = ""

    if(req.body.username.trim() == "") errors =["Invalid username / password"]
    if(req.body.password == "") errors = ["Invalid username / password"]

    if(errors.length){
        return res.render("login",{errors})
    }

    //compare information with database
    const userInQuestionStatement = db.prepare("SELECT * FROM users WHERE USERNAME = ?")
    const userInQuestion = userInQuestionStatement.get(req.body.username)

    if (!userInQuestion){
        errors = ["Invalid username / password."]
        return res.render("login",{errors})
    }

    const matchOrNot = bcrypt.compareSync(req.body.password,userInQuestion.password)
    if(!matchOrNot){
        errors = ["Invalid username / password."]
        return res.render("login",{errors})
    }

    //give user a cookie
    const TokenValue = jwt.sign({exp: Math.floor(Date.now()/1000) + 60 * 60 * 24, skyColor: "blue", userid: userInQuestion.id, username: userInQuestion.username}, process.env.JWTSECRET )
    res.cookie("userCookie", TokenValue, {
        hpptOnly: true,
        secure: true,
        sameSite: "strict",
        maxAge: 1000 * 60 * 60 *24
    })
    res.redirect("/")
    //redirect user to homepage

})
    //END

app.post("/register",(req,res) =>{
    const errors = []

    if(typeof req.body.username !== "string") req.body.username = ""
    if(typeof req.body.password !== "string") req.body.password = ""

    req.body.username = req.body.username.trim()

    //check for empty input, special characters, character limit and minimum
    if(!req.body.username) errors.push("Please enter username.")
    if(req.body.username && req.body.username.length < 3) errors.push("Username must be more than 3 characters long.")
    if(req.body.username && req.body.username.length > 10) errors.push("Username cannot exceed 10 characters.")
    if(req.body.username && !req.body.username.match(/^[a-zA-Z0-9]+$/)) errors.push("Username can only contain letters and nuumbers.")
    
    //check if username exists
        //COPY
    const usernameStatement = db.prepare("SELECT * FROM users WHERE username = ?")
    const usernameCheck = usernameStatement.get(req.body.username)
    if (usernameCheck) errors.push("That username is already taken.")
        //END

    //check for empty input, min max
    if(!req.body.password) errors.push("Please enter password.")
    if(req.body.password && req.body.password.length < 8) errors.push("Password must be more than 8 characters long.")
    if(req.body.password && req.body.password.length > 70) errors.push("Password cannot exceed 70 characters.")

    if(errors.length){
        return res.render("homepage",{errors})
    } 

    //save new user into database
        //COPY
    const salt = bcrypt.genSaltSync(10)
    req.body.password = bcrypt.hashSync(req.body.password, salt)
    const ourStatement = db.prepare("INSERT INTO users(username, password) VALUES(?, ?)")
    const result = ourStatement.run(req.body.username, req.body.password)
    
    const lookupStatement = db.prepare("SELECT * FROM users Where ROWID = ?")
    const ourUser = lookupStatement.get(result.lastInsertRowid)
        //END

    //log user in by giving a cookie
        //copy
    const TokenValue = jwt.sign({exp: Math.floor(Date.now()/1000) + 60 * 60 * 24, skyColor: "blue", userid: ourUser.id, username: ourUser.username}, process.env.JWTSECRET )
    res.cookie("userCookie", TokenValue, {
        hpptOnly: true,
        secure: true,
        sameSite: "strict",
        maxAge: 1000 * 60 * 60 *24
    })
    res.redirect("/")
        //END
})


app.listen(1000)