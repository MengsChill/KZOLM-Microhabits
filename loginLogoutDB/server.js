    //COPY
require("dotenv").config()
const sanitizeHTML = require('sanitize-html')
const jwt = require("jsonwebtoken")
const bcrypt = require("bcrypt")
const cookieParser = require('cookie-parser')
const express = require("express")
const db = require("better-sqlite3")("KZOLM.db")
db.pragma("journal_mode = WAL")
    //END 

// --- Date Helper Functions ---
function isSameDay(d1, d2) {
  return d1.getFullYear() === d2.getFullYear() &&
         d1.getMonth() === d2.getMonth() &&
         d1.getDate() === d2.getDate();
}
function isSameWeek(d1, d2) {
  const msPerDay = 1000 * 60 * 60 * 24;
  const startOfWeek1 = new Date(d1.getTime() - d1.getDay() * msPerDay);
  const startOfWeek2 = new Date(d2.getTime() - d2.getDay() * msPerDay);
  return isSameDay(startOfWeek1, startOfWeek2);
}
function isSameMonth(d1, d2) {
  return d1.getFullYear() === d2.getFullYear() &&
         d1.getMonth() === d2.getMonth();
}

function calculateStreak(habit, lastCompletion, today) {
  //If there's no last completion, this is the first one.
  if (!lastCompletion) {
    return 1;
  }

  const lastDate = new Date(lastCompletion.completionDate);
  const currentStreak = habit.streakCount;

  switch (habit.frequency) {
    case 'daily': {
      // Check if last completion was already today.
      if (isSameDay(lastDate, today)) {
        return currentStreak; // Already done, no change.
      }
      
      // Check if last completion was yesterday.
      const yesterday = new Date(today);
      yesterday.setDate(today.getDate() - 1);
      if (isSameDay(lastDate, yesterday)) {
        return currentStreak + 1; // Continue streak!
      }
      
      // If last completion wasn't today or yesterday, streak is broken.
      return 1;
    }

    case 'weekly': {
      // Check if last completion was already this week.
      if (isSameWeek(lastDate, today)) {
        return currentStreak; // Already done, no change.
      }

      // Check if last completion was last week.
      const lastWeek = new Date(today);
      lastWeek.setDate(today.getDate() - 7);
      if (isSameWeek(lastDate, lastWeek)) {
        return currentStreak + 1; // Continue streak!
      }

      // Streak is broken.
      return 1;
    }

    case 'monthly': {
      // Check if last completion was already this month.
      if (isSameMonth(lastDate, today)) {
        return currentStreak; // Already done, no change.
      }

      // Check if last completion was last month.
      const lastMonth = new Date(today);
      lastMonth.setMonth(today.getMonth() - 1);
      if (isSameMonth(lastDate, lastMonth)) {
        return currentStreak + 1; // Continue streak!
      }
      
      // Streak is broken.
      return 1;
    }

    default:
      // Fallback for unknown frequencies
      return 1;
  }
}

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

db.prepare(
    `
    CREATE TABLE IF NOT EXISTS habits(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    createdDate TEXT,
    habitName STRING NOT NULL,
    frequency STRING NOT NULL,
    streakCount INTEGER DEFAULT 0,
    authorID INTEGER,
    FOREIGN KEY (authorID) REFERENCES users (id)
    )
    `
).run()

db.prepare(
    `
    CREATE TABLE IF NOT EXISTS completions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    habitID INTEGER,
    completionDate TEXT NOT NULL,
    FOREIGN KEY (habitID) REFERENCES habits (id)
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
    
app.get("/", (req, res) => {
    if (req.user) {
        //if user is logged in, fetch their habits
        const habits = db.prepare("SELECT * FROM habits WHERE authorID = ?")
                         .all(req.user.userid);
                         
        //Pass the habits list to the dashboard
        return res.render("dashboard", { habits: habits });
    }
    //Not logged in, show homepage
    res.render("homepage",{errors:[]});
});

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

function loggedIn(req,res,next){
    if(req.user){
        return next()
    }
    return res.redirect("/")
}

app.get("/post/:id",(req,res) => {
    const statement = db.prepare("SELECT * FROM habits WHERE id = ?")
    const habit = statement.get(req.params.id)

    if (!habit){
        return res.redirect("/")
    }
    
    // Check if completed today for display purposes
    const today = new Date().toISOString().substring(0, 10);
    const completionCheckStatement = db.prepare("SELECT * FROM completions WHERE habitID = ? AND completionDate LIKE ? || '%'");
    const completedToday = !!completionCheckStatement.get(req.params.id, today);

    // Pass data to template
    res.render("single-habit",{
        habit, 
        completedToday, 
        streak: habit.streakCount // Assumes habit now has a streakCount field
    })
})

app.get("/add-habit",loggedIn,(req,res) =>{
    res.render("add-habit")
})

function addedHabitValidation(req){
    const errors =[]

    if(typeof req.body.habit !== "string") req.body.habit = ""
    if(typeof req.body.frequency !== "string") req.body.frequency = ""

    //trim - sanitize or strip out html
    req.body.habit = sanitizeHTML(req.body.habit.trim(),{allowedTags:[], allowedAtributes:{}})
    req.body.frequency = sanitizeHTML(req.body.frequency.trim(),{allowedTags:[], allowedAtributes:{}})

    if(!req.body.habit) errors.push("You must provide a habit name.")

    //check for frequency
    const validFrequencies = ['daily','weekly','monthly']
    if(!validFrequencies.includes(req.body.frequency)) errors.push("Invalid frequency selected.")

    return errors
}

app.post("/add-habit", loggedIn, (req,res) => {
    const errors = addedHabitValidation(req)

    if(errors.length){
        return res.render("add-habit",{errors})
    }

    //save into database
    const ourStatement = db.prepare("INSERT INTO habits (habitName,frequency,authorID,createdDate) VALUES (?,?,?,?)")
    const result = ourStatement.run(req.body.habit,req.body.frequency,req.user.userid,new Date().toISOString())

    const getHabitStatement = db.prepare("SELECT * FROM habits WHERE ROWID = ?")
    const realHabit = getHabitStatement.get(result.lastInsertRowid)

    res.redirect(`/post/${realHabit.id}`)
})

app.get("/edit-habit/:id", loggedIn, (req, res) => {
    //Find the habit in the database
    const habitStatement = db.prepare("SELECT * FROM habits WHERE id = ? AND authorID = ?");
    const habit = habitStatement.get(req.params.id, req.user.userid);

    //Check if the habit exists and belongs to the user
    if (!habit) {
        return res.redirect("/"); // Not found or not theirs
    }

    //Render the edit view, passing the habit data
    res.render("edit-habit", { habit: habit, errors: [] });
})

app.post("/edit-habit/:id", loggedIn, (req, res) => {
    const habitId = req.params.id;
    const userId = req.user.userid;

    //Validate the new input
    const errors = addedHabitValidation(req); // Reuse your existing validation function

    if (errors.length) {
        const habitStatement = db.prepare("SELECT * FROM habits WHERE id = ? AND authorID = ?");
        const habit = habitStatement.get(habitId, userId);
        
        if (!habit) return res.redirect("/"); // In case something went wrong
        
        return res.render("edit-habit", { habit: habit, errors: errors });
    }

    //Update the database
    const updateStatement = db.prepare("UPDATE habits SET habitName = ?, frequency = ? WHERE id = ? AND authorID = ?");
    const result = updateStatement.run(req.body.habit, req.body.frequency, habitId, userId);

    //Redirect back to the single habit page
    res.redirect(`/post/${habitId}`);
})

app.post("/complete-habit/:id", loggedIn, (req, res) => {
    const habitId = req.params.id;
    const userId = req.user.userid;
    const today = new Date(); // Use one consistent "today" date

    //Get the habit and check ownership
    const habitStatement = db.prepare("SELECT * FROM habits WHERE id = ? AND authorID = ?");
    const habit = habitStatement.get(habitId, userId);

    if (!habit) {
        return res.status(404).send("Habit not found or unauthorized.");
    }

    //Get the most recent completion for this habit
    const lastCompletionStatement = db.prepare("SELECT * FROM completions WHERE habitID = ? ORDER BY completionDate DESC LIMIT 1");
    const lastCompletion = lastCompletionStatement.get(habitId);

    //Calculate the new streak
    const newStreak = calculateStreak(habit, lastCompletion, today);

    //Check if we've already completed this period to avoid duplicate logs
    let completedThisPeriod = false;
    if (lastCompletion) {
        const lastDate = new Date(lastCompletion.completionDate);
        if (habit.frequency === 'daily' && isSameDay(lastDate, today)) completedThisPeriod = true;
        if (habit.frequency === 'weekly' && isSameWeek(lastDate, today)) completedThisPeriod = true;
        if (habit.frequency === 'monthly' && isSameMonth(lastDate, today)) completedThisPeriod = true;
    }

    //If not completed, log the new completion
    if (!completedThisPeriod) {
        db.prepare("INSERT INTO completions (habitID, completionDate) VALUES (?, ?)")
          .run(habitId, today.toISOString());
    }

    //Update the habit's streak count in the database
    db.prepare("UPDATE habits SET streakCount = ? WHERE id = ?")
      .run(newStreak, habitId);

    res.redirect(`/post/${habitId}`);
})

app.post("/delete-habit/:id", loggedIn, (req, res) => {
    const habitId = req.params.id;
    const userId = req.user.userid;

    // must use a transaction to delete from two tables.
    //must delete from 'completions' first to avoid a 'FOREIGN KEY constraint failed' error.
    const deleteTransaction = db.transaction(() => {
        // Delete all completions associated with the habit
        db.prepare("DELETE FROM completions WHERE habitID = ?")
          .run(habitId);

        // Delete the habit itself (checking for ownership)
        const info = db.prepare("DELETE FROM habits WHERE id = ? AND authorID = ?")
                       .run(habitId, userId);

        return info; // Return the result of the deletion
    });

    // Run the transaction
    const result = deleteTransaction();

    //'result.changes' will be 1 if the habit was found and deleted, or 0 if not.
    if (result.changes === 0) {
        return res.redirect("/");
    }

    //Redirect to the dashboard
    res.redirect("/");
});

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