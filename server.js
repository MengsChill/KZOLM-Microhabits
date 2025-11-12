// Import required modules
const express = require("express");
const path = require("path");
const sqlite3 = require("sqlite3").verbose();
const bodyParser = require("body-parser");
const bcrypt = require("bcrypt");

// Initialize Express app
const app = express();
const PORT = 3000;

// Middleware
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

// Serve static files from "public" folder
app.use(express.static(path.join(__dirname, "public")));

// Connect to SQLite database (or create it if not exists)
const db = new sqlite3.Database("./users.db", (err) => {
  if (err) {
    console.error("❌ Database connection failed:", err.message);
  } else {
    console.log("✅ Connected to SQLite database.");
  }
});


//database preparation
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

// ✅ Route: Home page
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "public/html/index.html"));
});

// ✅ Route: Sign in page
app.get("/signin", (req, res) => {
  res.sendFile(path.join(__dirname, "public/html/signin.html"));
});

// ✅ Route: Sign up page
app.get("/signup", (req, res) => {
  res.sendFile(path.join(__dirname, "public/html/signup.html"));
});

// ✅ Route: Menu page
app.get("/menu", (req, res) => {
  res.sendFile(path.join(__dirname, "public/html/menu.html"));
});

// ✅ POST route: handle user signup
app.post("/signup", async (req, res) => {
  const { email, password } = req.body;

  if (!email || !password) {
    return res.status(400).send("Please fill all fields.");
  }

  try {
    const hashedPassword = await bcrypt.hash(password, 10);
    db.run(
      `INSERT INTO users (email, password) VALUES (?, ?)`,
      [email, hashedPassword],
      (err) => {
        if (err) {
          console.error("Signup error:", err.message);
          res.status(400).send("User already exists or invalid data.");
        } else {
          res.redirect("/signin");
        }
      }
    );
  } catch (error) {
    console.error("Error during signup:", error);
    res.status(500).send("Internal server error.");
  }
});

// ✅ POST route: handle user signin
app.post("/signin", (req, res) => {
  const { email, password } = req.body;

  db.get(`SELECT * FROM users WHERE email = ?`, [email], async (err, user) => {
    if (err) {
      console.error("Database error:", err.message);
      return res.status(500).send("Internal server error.");
    }
    if (!user) {
      return res.status(401).send("User not found.");
    }

    const match = await bcrypt.compare(password, user.password);
    if (match) {
      res.redirect("/menu");
    } else {
      res.status(401).send("Incorrect password.");
    }
  });
});

// ✅ Start the server
app.listen(PORT, () => {
  console.log(`🚀 Server running at http://localhost:${PORT}`);
});
