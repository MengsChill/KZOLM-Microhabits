const express = require("express")
const path = require("path")
const sqlite3 = require("sqlite3").verbose()
const bodyParser = require("body-parser")
const bcrypt = require("bcrypt")
const session = require("express-session")
const multer = require("multer")
const fs = require("fs")

const app = express()
const PORT = 3000

app.use(bodyParser.urlencoded({ extended: true }))
app.use(bodyParser.json())

app.use(session({
    secret: 'suganigadih',
    resave: false,
    saveUninitialized: false,
    rolling: true,
    cookie: { maxAge: 3600000 }
}))

app.use(express.static(path.join(__dirname, "public")))

const uploadDir = path.join(__dirname, "public/uploads");
if (!fs.existsSync(uploadDir)){
    fs.mkdirSync(uploadDir, { recursive: true });
}

const storage = multer.diskStorage({
    destination: function (req, file, cb) {
        cb(null, uploadDir)
    },
    filename: function (req, file, cb) {
        const uniqueSuffix = Date.now() + '-' + Math.round(Math.random() * 1E9)
        cb(null, 'user-' + req.session.userId + '-' + uniqueSuffix + path.extname(file.originalname))
    }
})

const upload = multer({ storage: storage })

const db = new sqlite3.Database("./users.db", (err) => {
    if (err) console.error("❌ Database connection failed:", err.message)
    else console.log("✅ Connected to SQLite database.")
})

db.run(`
  CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    email TEXT UNIQUE,
    password TEXT,
    display_name TEXT DEFAULT 'User',
    profile_image TEXT,
    banner_image TEXT,
    bio TEXT
  )
`)

db.run(`
  CREATE TABLE IF NOT EXISTS posts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    content TEXT NOT NULL,
    image_url TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
  )
`)

db.run(`
  CREATE TABLE IF NOT EXISTS comments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    post_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    content TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (post_id) REFERENCES posts (id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
  )
`)

db.run(`
  CREATE TABLE IF NOT EXISTS likes (
    user_id INTEGER NOT NULL,
    post_id INTEGER NOT NULL,
    PRIMARY KEY (user_id, post_id),
    FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE,
    FOREIGN KEY (post_id) REFERENCES posts (id) ON DELETE CASCADE
  )
`)
const dbGet = (sql, params = []) => {
    return new Promise((resolve, reject) => {
        db.get(sql, params, (err, row) => {
            if (err) return reject(err);
            resolve(row);
        })
    })
}

const dbAll = (sql, params = []) => {
    return new Promise((resolve, reject) => {
        db.all(sql, params, (err, rows) => {
            if (err) return reject(err);
            resolve(rows);
        })
    })
}

const dbRun = (sql, params = []) => {
    return new Promise(function(resolve, reject) {
        db.run(sql, params, function(err) {
            if (err) return reject(err);
            resolve({ lastID: this.lastID, changes: this.changes });
        })
    })
}

app.get("/", (req, res) => res.sendFile(path.join(__dirname, "public/html/index.html")))
app.get("/signin", (req, res) => res.sendFile(path.join(__dirname, "public/html/signin.html")))
app.get("/signup", (req, res) => res.sendFile(path.join(__dirname, "public/html/signup.html")))
app.get("/progress", (req, res) => res.sendFile(path.join(__dirname, "public/html/progress.html")))
app.get("/community", (req, res) => res.sendFile(path.join(__dirname, "public/html/community.html")))
app.get("/add-habit", (req, res) => res.sendFile(path.join(__dirname, "public/html/addhabit.html")))
app.get("/explore-now", (req, res) => res.sendFile(path.join(__dirname, "public/html/mainpage.html")))


//menu
app.get("/menu", (req, res) => {
    if (!req.session.userId) return res.redirect("/signin")
    res.sendFile(path.join(__dirname, "public/html/dashboard.html"))
})

//profile
app.get("/profile", (req, res) => {
    if (!req.session.userId) return res.redirect("/signin")
    res.sendFile(path.join(__dirname, "public/html/profile.html"))
})

//edit profile
app.get("/edit-profile.html", (req, res) => {
    if (!req.session.userId) return res.redirect("/signin")
    res.sendFile(path.join(__dirname, "public/html/edit-profile.html"))
})

//signup
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

//signin
app.post("/signin", (req, res) => {
    const { email, password } = req.body
    db.get(`SELECT * FROM users WHERE email = ?`, [email], async (err, user) => {
        if (err || !user) return res.status(401).send("User not found.")
        const match = await bcrypt.compare(password, user.password)
        if (match) {
            req.session.userId = user.id
            res.redirect("/menu")
        } else {
            res.status(401).send("Incorrect password.")
        }
    })
})
app.get("/api/user/:id", (req, res) => {
    const userId = req.params.id;
    db.get("SELECT id, display_name, profile_image, banner_image, bio FROM users WHERE id = ?", [userId], (err, row) => {
        if (err) return res.status(500).json({ error: "Database error" })
        if (!row) return res.status(404).json({ error: "User not found" })
        res.json(row)
    })
})

// COMMUNITY API's

// Middleware to check for authentication
const ensureAuth = (req, res, next) => {
    if (req.session.userId) {
        next();
    } else {
        res.status(401).json({ error: "Unauthorized" });
    }
};

// GET ALL POSTS
app.get("/api/posts", ensureAuth, async (req, res) => {
    try {
        // 1. Get all posts, joining with user info for the author
        const posts = await dbAll(`
            SELECT 
                p.id, p.content, p.image_url, strftime('%Y-%m-%dT%H:%M:%SZ', p.created_at) AS created_at,
                u.id as author_id, u.display_name as author_name, u.profile_image as author_image
            FROM posts p
            JOIN users u ON p.user_id = u.id
            ORDER BY p.created_at DESC
        `)

        //for each post show likes etc.
        const enrichedPosts = await Promise.all(posts.map(async (post) => {
            //get comments
        const comments = await dbAll(`
                SELECT 
                    c.id, c.content, strftime('%Y-%m-%dT%H:%M:%SZ', c.created_at) AS created_at,
                    u.id as author_id, u.display_name as author_name, u.profile_image as author_image
                FROM comments c
                JOIN users u ON c.user_id = u.id
                WHERE c.post_id = ?
                ORDER BY c.created_at ASC
            `, [post.id])

            //get like count
            const { likeCount } = await dbGet(`SELECT COUNT(*) as likeCount FROM likes WHERE post_id = ?`, [post.id]);

            //check if current user liked post
            const userLiked = await dbGet(`SELECT 1 FROM likes WHERE post_id = ? AND user_id = ?`, [post.id, req.session.userId]);

            return {
                ...post,
                comments,
                likes: likeCount,
                liked: !!userLiked
            }
        }))

        res.json(enrichedPosts)

    } catch (err) {
        console.error(err)
        res.status(500).json({ error: "Failed to fetch posts" })
    }
});

//create new post
app.post("/api/posts", ensureAuth, upload.single('image'), async (req, res) => {
    const { content } = req.body
    const userId = req.session.userId
    const imageUrl = req.file ? req.file.filename : null;

    if (!content) {
        return res.status(400).json({ error: "Post content is required" });
    }

    try {
        const result = await dbRun(
            `INSERT INTO posts (user_id, content, image_url) VALUES (?, ?, ?)`,
            [userId, content, imageUrl]
        );
        res.status(201).json({ success: true, postId: result.lastID });
    } catch (err) {
        console.error(err);
        res.status(500).json({ error: "Failed to create post" });
    }
});

//toggle likes on post
app.post("/api/posts/:id/like", ensureAuth, async (req, res) => {
    const postId = req.params.id;
    const userId = req.session.userId;

    try {
        //check if user liked
        const liked = await dbGet(`SELECT * FROM likes WHERE user_id = ? AND post_id = ?`, [userId, postId]);

        if (liked) {
            //unlike
            await dbRun(`DELETE FROM likes WHERE user_id = ? AND post_id = ?`, [userId, postId]);
            res.json({ success: true, liked: false });
        } else {
            //like
            await dbRun(`INSERT INTO likes (user_id, post_id) VALUES (?, ?)`, [userId, postId]);
            res.json({ success: true, liked: true });
        }
    } catch (err) {
        console.error(err);
        res.status(500).json({ error: "Failed to toggle like" });
    }
});

//add comment
app.post("/api/posts/:id/comments", ensureAuth, async (req, res) => {
    const postId = req.params.id;
    const userId = req.session.userId;
    const { content } = req.body;

    if (!content) {
        return res.status(400).json({ error: "Comment content is required" });
    }

    try {
        const result = await dbRun(
            `INSERT INTO comments (post_id, user_id, content) VALUES (?, ?, ?)`,
            [postId, userId, content]
        );
        res.status(201).json({ success: true, commentId: result.lastID });
    } catch (err) {
        console.error(err);
        res.status(500).json({ error: "Failed to add comment" });
    }
});

//delete post
app.delete("/api/posts/:id", ensureAuth, async (req, res) => {
    const postId = req.params.id;
    const userId = req.session.userId;

    try {
        //verify the user owns the post
        const post = await dbGet(`SELECT user_id FROM posts WHERE id = ?`, [postId]);
        if (!post) return res.status(404).json({ error: "Post not found" });
        
        if (post.user_id !== userId) {
            return res.status(403).json({ error: "You are not authorized to delete this post" });
        }

        await dbRun(`DELETE FROM posts WHERE id = ?`, [postId]);
        res.json({ success: true });
    } catch (err) {
        console.error(err);
        res.status(500).json({ error: "Failed to delete post" });
    }
});

//delete comment
app.delete("/api/comments/:id", ensureAuth, async (req, res) => {
    const commentId = req.params.id;
    const userId = req.session.userId;

    try {
        //verify if user owns comment
        const comment = await dbGet(`SELECT user_id FROM comments WHERE id = ?`, [commentId]);
        if (!comment) return res.status(404).json({ error: "Comment not found" });

        if (comment.user_id !== userId) {
            return res.status(403).json({ error: "You are not authorized to delete this comment" });
        }

        await dbRun(`DELETE FROM comments WHERE id = ?`, [commentId]);
        res.json({ success: true });
    } catch (err) {
        console.error(err);
        res.status(500).json({ error: "Failed to delete comment" });
    }
})

//user API's
app.get("/api/user", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    
    db.get("SELECT email, display_name, profile_image, banner_image, bio FROM users WHERE id = ?", [req.session.userId], (err, row) => {
        if (err) return res.status(500).json({ error: "Database error" })
        res.json(row)
    })
})

app.post("/api/verify-password", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    db.get("SELECT password FROM users WHERE id = ?", [req.session.userId], async (err, row) => {
        if (err || !row) return res.status(500).json({ error: "Error" })
        const match = await bcrypt.compare(req.body.password, row.password)
        if (match) res.json({ success: true })
        else res.status(401).json({ success: false })
    })
})

app.put("/api/update-profile", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    const { display_name, email, bio } = req.body
    db.run("UPDATE users SET display_name = ?, email = ?, bio = ? WHERE id = ?", 
        [display_name, email, bio, req.session.userId], 
        (err) => {
            if (err) return res.status(500).json({ error: err.message })
            res.json({ success: true })
        }
    )
})

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

app.delete("/api/delete-account", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    db.run("DELETE FROM users WHERE id = ?", [req.session.userId], () => {
        req.session.destroy()
        res.json({ success: true })
    })
})

app.post("/api/upload-avatar", upload.single('croppedImage'), (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    if (!req.file) return res.status(400).json({ error: "No file uploaded" })

    const filename = req.file.filename;
    db.run("UPDATE users SET profile_image = ? WHERE id = ?", [filename, req.session.userId], (err) => {
        if (err) return res.status(500).json({ error: "Database update failed" })
        res.json({ success: true, filename: filename })
    })
})

app.post("/api/upload-banner", upload.single('croppedImage'), (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    if (!req.file) return res.status(400).json({ error: "No file uploaded" })

    const filename = req.file.filename;
    db.run("UPDATE users SET banner_image = ? WHERE id = ?", [filename, req.session.userId], (err) => {
        if (err) return res.status(500).json({ error: "Database update failed" })
        res.json({ success: true, filename: filename })
    })
})

app.listen(PORT, () => console.log(`🚀 Server running at http://localhost:${PORT}`))