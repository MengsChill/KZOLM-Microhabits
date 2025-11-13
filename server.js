// Import required modules
const express = require("express")
const path = require("path")
const sqlite3 = require("sqlite3").verbose()
const bodyParser = require("body-parser")
const bcrypt = require("bcrypt")
const session = require("express-session")
const multer = require("multer")
const fs = require("fs")

// Initialize Express app
const app = express()
const PORT = 3000

// Middleware
app.use(bodyParser.urlencoded({ extended: true }))
app.use(bodyParser.json())

// Session config (keeps users logged in)
app.use(session({
    secret: 'suganigadih',
    resave: false,
    saveUninitialized: false, // FIXED TYPO (Capital U)
    rolling: true, // Resets timer on activity
    cookie: { maxAge: 3600000 } // 1 hour
}))

// Serve static files from "public" folder
app.use(express.static(path.join(__dirname, "public")))

//configure file upload storage
//make sure uploads folder exists
const uploadDir = path.join(__dirname, "public/uploads");
if (!fs.existsSync(uploadDir)){
    fs.mkdirSync(uploadDir, { recursive: true });
}

//configure multer
const storage = multer.diskStorage({
    destination: function (req, file, cb) {
        cb(null, uploadDir) // Save to public/uploads
    },
    filename: function (req, file, cb) {
        // Save as "user-ID-timestamp.jpg" to avoid duplicate names
        const uniqueSuffix = Date.now() + '-' + Math.round(Math.random() * 1E9)
        cb(null, 'user-' + req.session.userId + '-' + uniqueSuffix + path.extname(file.originalname))
    }
})
const upload = multer({ storage: storage })

// Connect to SQLite database
const db = new sqlite3.Database("./users.db", (err) => {
    if (err) {
        console.error("❌ Database connection failed:", err.message)
    } else {
        console.log("✅ Connected to SQLite database.")
    }
})

// Initialize Table
db.run(`
  CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    email TEXT UNIQUE,
    password TEXT,
    display_name TEXT DEFAULT 'User',  
    profile_image TEXT                
  )
`)

// --- VIEW ROUTES ---

app.get("/", (req, res) => res.sendFile(path.join(__dirname, "public/html/index.html")))
app.get("/signin", (req, res) => res.sendFile(path.join(__dirname, "public/html/signin.html")))
app.get("/signup", (req, res) => res.sendFile(path.join(__dirname, "public/html/signup.html")))
app.get("/progress", (req, res) => res.sendFile(path.join(__dirname, "public/html/progress.html")))
app.get("/community", (req, res) => res.sendFile(path.join(__dirname, "public/html/community.html")))
app.get("/add-habit", (req, res) => res.sendFile(path.join(__dirname, "public/html/addhabit.html")))
app.get("/explore-now", (req, res) => res.sendFile(path.join(__dirname, "public/html/mainpage.html")))

// Protected Routes (Redirect to signin if not logged in)
app.get("/menu", (req, res) => {
    if (!req.session.userId) return res.redirect("/signin") // Fixed variable name
    res.sendFile(path.join(__dirname, "public/html/dashboard.html"))
})

app.get("/profile", (req, res) => {
    if (!req.session.userId) return res.redirect("/signin") // Fixed variable name
    res.sendFile(path.join(__dirname, "public/html/profile.html"))
})

// --- AUTHENTICATION ROUTES ---

// Sign Up
app.post("/signup", async (req, res) => {
    const { email, password } = req.body

    if (!email || !password) return res.status(400).send("Please fill all fields.")

    try {
        const hashedPassword = await bcrypt.hash(password, 10)
        db.run(
            `INSERT INTO users (email, password, display_name) VALUES (?, ?, ?)`,
            [email, hashedPassword, "User"],
            (err) => {
                if (err) {
                    console.error("Signup error:", err.message)
                    res.status(400).send("User already exists.")
                } else {
                    res.redirect("/signin")
                }
            }
        )
    } catch (error) {
        res.status(500).send("Internal server error.")
    }
})

// Sign In
app.post("/signin", (req, res) => {
    const { email, password } = req.body

    db.get(`SELECT * FROM users WHERE email = ?`, [email], async (err, user) => {
        if (err || !user) return res.status(401).send("User not found.")

        const match = await bcrypt.compare(password, user.password)
        if (match) {
            req.session.userId = user.id // FIXED variable name (lowercase d)
            res.redirect("/menu")
        } else {
            res.status(401).send("Incorrect password.")
        }
    })
})

//--------profile API routes

//Get User Info
app.get("/api/user", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    
    db.get("SELECT email, display_name, profile_image FROM users WHERE id = ?", [req.session.userId], (err, row) => {
        if (err) return res.status(500).json({ error: "Database error" })
        res.json(row)
    })
})

//Verify Password
app.post("/api/verify-password", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    db.get("SELECT password FROM users WHERE id = ?", [req.session.userId], async (err, row) => {
        if (err || !row) return res.status(500).json({ error: "Error" })
        const match = await bcrypt.compare(req.body.password, row.password)
        if (match) res.json({ success: true })
        else res.status(401).json({ success: false })
    })
})

//Update Profile Info
app.put("/api/update-profile", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    const { display_name, email } = req.body
    db.run("UPDATE users SET display_name = ?, email = ? WHERE id = ?", 
        [display_name, email, req.session.userId], 
        (err) => {
            if (err) return res.status(500).json({ error: err.message })
            res.json({ success: true })
        }
    )
})

//Change Password
app.put("/api/change-password", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    const { currentPassword, newPassword } = req.body
    db.get("SELECT password FROM users WHERE id = ?", [req.session.userId], async (err, row) => {
        if (!await bcrypt.compare(currentPassword, row.password)) {
            return res.status(401).json({ error: "Incorrect password" })
        }
        const newHashed = await bcrypt.hash(newPassword, 10)
        db.run("UPDATE users SET password = ? WHERE id = ?", [newHashed, req.session.userId], 
            () => res.json({ success: true })
        )
    })
})

//Delete Account
app.delete("/api/delete-account", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    db.run("DELETE FROM users WHERE id = ?", [req.session.userId], () => {
        req.session.destroy()
        res.json({ success: true })
    })
})

//Upload Avatar Route
app.post("/api/upload-avatar", upload.single('avatar'), (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    if (!req.file) return res.status(400).json({ error: "No file uploaded" })
    const filename = req.file.filename;
    db.run("UPDATE users SET profile_image = ? WHERE id = ?", [filename, req.session.userId], (err) => {
        if (err) return res.status(500).json({ error: "Database update failed" })
        res.json({ success: true, filename: filename })
    })
})

// Start server
app.listen(PORT, () => console.log(`🚀 Server running at http://localhost:${PORT}`))