const express = require("express")
const path = require("path")
const sqlite3 = require("sqlite3").verbose()
const bodyParser = require("body-parser")
const bcrypt = require("bcrypt")
const session = require("express-session")
const multer = require("multer")
const fs = require("fs")

const app = express()
const PORT = 3001

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
        if (!req.session || !req.session.userId) {
            // Handle case where user is not logged in or session is not populated
            return cb(new Error("User not authenticated"), false);
        }
        const uniqueSuffix = Date.now() + '-' + Math.round(Math.random() * 1E9)
        cb(null, 'user-' + req.session.userId + '-' + uniqueSuffix + path.extname(file.originalname))
    }
})

const upload = multer({ storage: storage })

// --- FIX: Use Absolute Path to ensure we use the REAL database ---
const dbPath = path.join(__dirname, "users.db");
const db = new sqlite3.Database(dbPath, (err) => {
    if (err) console.error("❌ Database connection failed:", err.message)
    else console.log("✅ Connected to SQLite database.")
})


//db setup
db.serialize(() => {
    // --- CREATE ALL TABLES FIRST ---
db.run(`
  CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    email TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    security_question TEXT,
    security_answer TEXT,
    display_name TEXT DEFAULT 'User',
    profile_image TEXT,
    banner_image TEXT,
    bio TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP  -- <--- ADD THIS LINE
  )
`);

    db.run(`
      CREATE TABLE IF NOT EXISTS posts (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        content TEXT NOT NULL,
        image_url TEXT,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
      )
    `);

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
    `);

    db.run(`
      CREATE TABLE IF NOT EXISTS likes (
        user_id INTEGER NOT NULL,
        post_id INTEGER NOT NULL,
        PRIMARY KEY (user_id, post_id),
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE,
        FOREIGN KEY (post_id) REFERENCES posts (id) ON DELETE CASCADE
      )
    `);

    db.run(`
      CREATE TABLE IF NOT EXISTS tasks (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        name TEXT NOT NULL,
        description TEXT,
        category TEXT DEFAULT 'personal',
        priority TEXT DEFAULT 'medium',
        frequency TEXT DEFAULT 'once',
        custom_days TEXT,
        is_completed BOOLEAN DEFAULT 0,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        completed_at DATETIME,
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
      )
    `);
    
    db.run(`
      CREATE TABLE IF NOT EXISTS user_stats (
        user_id INTEGER PRIMARY KEY,
        total_points REAL DEFAULT 0,
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE
      )
    `);
    
    db.run(`
      CREATE TABLE IF NOT EXISTS daily_completions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        task_id INTEGER NOT NULL,
        completed_date TEXT NOT NULL,
        points_awarded REAL NOT NULL,
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE,
        FOREIGN KEY (task_id) REFERENCES tasks (id) ON DELETE CASCADE
      )
    `);
    
    db.run(`
      CREATE TABLE IF NOT EXISTS achievements (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT UNIQUE NOT NULL,
        description TEXT NOT NULL,
        type TEXT NOT NULL,
        threshold REAL NOT NULL,
        icon TEXT NOT NULL
      )
    `);
    
    db.run(`
      CREATE TABLE IF NOT EXISTS user_achievements (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        achievement_id INTEGER NOT NULL,
        unlocked_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        is_displayed BOOLEAN DEFAULT 0,
        FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE,
        FOREIGN KEY (achievement_id) REFERENCES achievements (id) ON DELETE CASCADE,
        UNIQUE(user_id, achievement_id)
      )
    `, (err) => { // <-- Callback on the *LAST* table creation
        if (err) {
            console.error("CRITICAL: Error creating tables:", err.message);
            return;
        }
        console.log("✅ All tables created or verified.");

        // --- Force-delete all existing achievements to clear corruption ---
        db.run("DELETE FROM achievements", (deleteErr) => {
            if (deleteErr) {
                console.error("CRITICAL: Error clearing achievements table:", deleteErr.message);
                return;
            }
            console.log("✅ Cleared old achievements.");
            
            // --- Seeding logic runs *after* tables are created and cleared ---
            const achievements = [
                { name: 'Beginner', desc: 'Reach 10 points', type: 'points', threshold: 10, icon: 'fas fa-seedling' },
                { name: 'Learner', desc: 'Reach 50 points', type: 'points', threshold: 50, icon: 'fas fa-leaf' },
                { name: 'Explorer', desc: 'Reach 150 points', type: 'points', threshold: 150, icon: 'fas fa-feather' },
                { name: 'Adventurer', desc: 'Reach 450 points', type: 'points', threshold: 450, icon: 'fas fa-hiking' },
                { name: 'Climber', desc: 'Reach 900 points', type: 'points', threshold: 900, icon: 'fas fa-mountain' },
                { name: 'Expert', desc: 'Reach 1500 points', type: 'points', threshold: 1500, icon: 'fas fa-tree' },
                { name: 'Master', desc: 'Reach 3000 points', type: 'points', threshold: 3000, icon: 'fas fa-trophy' }
            ];

            const stmt = db.prepare("INSERT OR IGNORE INTO achievements (name, description, type, threshold, icon) VALUES (?, ?, ?, ?, ?)");
        
            let insertCount = 0;
            let hasInsertError = null;

            function insertRow(index) {
                if (index >= achievements.length) {
                    stmt.finalize((finalizeErr) => {
                        if (finalizeErr) {
                            console.error("CRITICAL: Error finalizing statement:", finalizeErr.message);
                            return;
                        }
                        if (hasInsertError) {
                            console.error("CRITICAL: Server NOT started due to errors inserting achievements.");
                            return;
                        }
                        console.log(`✅ ${insertCount} achievements seeded successfully.`);
                        
                        // *** IT'S NOW SAFE TO START THE SERVER ***
                        initializeApp(); 
                    });
                    return;
                }

                const ach = achievements[index];
                stmt.run(ach.name, ach.desc, ach.type, ach.threshold, ach.icon, function(insertErr) {
                    if (insertErr) {
                        console.error("CRITICAL: Error inserting achievement:", ach.name, insertErr.message);
                        hasInsertError = insertErr;
                    } else {
                        if (this.changes > 0) {
                            insertCount++;
                        }
                    }
                    insertRow(index + 1);
                });
            }
            
            // Start the sequential insert
            insertRow(0);

        }); // --- END OF DELETE WRAPPER ---
    }); // --- END OF *LAST* TABLE CALLBACK ---
}); // --- END OF *OUTER* DB.SERIALIZE BLOCK ---


//
// All server logic is wrapped in this function
//
function initializeApp() {

    console.log("\n\n--- SERVER INITIALIZEAPP() IS RUNNING ---");

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

    // --- HELPER FUNCTIONS ---

    const calculateStreak = async (userId) => {
        const completedDates = await dbAll(`
            SELECT DISTINCT date(completed_at) AS date
            FROM tasks
            WHERE user_id = ? AND completed_at IS NOT NULL
            ORDER BY date DESC
        `, [userId]);

        let currentStreak = 0;
        if (completedDates.length === 0) return 0;

        let today = new Date();
        let todayStr = today.toISOString().split('T')[0];
        
        let lastCompletionDateStr = completedDates[0].date;
        
        if (lastCompletionDateStr === todayStr) {
            currentStreak = 1;
            today.setDate(today.getDate() - 1);
        } else {
            today.setDate(today.getDate() - 1);
            let yesterdayStr = today.toISOString().split('T')[0];
            if(lastCompletionDateStr === yesterdayStr) {
                currentStreak = 1;
                today.setDate(today.getDate() - 1);
            } else {
                return 0;
            }
        }

        for (let i = 1; i < completedDates.length; i++) {
            let prevDayStr = today.toISOString().split('T')[0];
            if (completedDates[i].date === prevDayStr) {
                currentStreak++;
                today.setDate(today.getDate() - 1);
            } else {
                break;
            }
        }
        
        return currentStreak;
    };

    const checkAndAwardAchievements = async (userId) => {
        try {
            const stats = await dbGet('SELECT total_points FROM user_stats WHERE user_id = ?', [userId]);
            if (!stats) return;

            const { total_points } = stats;

            const existing = await dbAll('SELECT achievement_id FROM user_achievements WHERE user_id = ?', [userId]);
            const existingIds = existing.map(e => e.achievement_id);

            const newAchievements = await dbAll(`
                SELECT id FROM achievements
                WHERE type = 'points' AND threshold <= ? AND id NOT IN (${existingIds.join(',') || 0})
            `, [total_points]);

            for (const ach of newAchievements) {
                await dbRun("INSERT OR IGNORE INTO user_achievements (user_id, achievement_id) VALUES (?, ?)", [userId, ach.id]);
            }
        } catch (err) {
            console.error("Failed to check achievements:", err);
        }
    };


    // --- END HELPER FUNCTIONS ---


    app.get("/", (req, res) => res.sendFile(path.join(__dirname, "public/html/index.html")))
    app.get("/signin", (req, res) => res.sendFile(path.join(__dirname, "public/html/signin.html")))
    app.get("/signup", (req, res) => res.sendFile(path.join(__dirname, "public/html/signup.html")))
    app.get("/progress", (req, res) => res.sendFile(path.join(__dirname, "public/html/progress.html")))
    app.get("/community", (req, res) => res.sendFile(path.join(__dirname, "public/html/community.html")))
    app.get("/add-habit", (req, res) => res.sendFile(path.join(__dirname, "public/html/addhabit.html")))
    app.get("/main", (req, res) => res.sendFile(path.join(__dirname, "public/html/mainpage.html")))

    app.get("/milestone", (req, res) => {
        if (!req.session.userId) return res.redirect("/signin")
        res.sendFile(path.join(__dirname, "public/html/milestone.html"))
    })

    app.get("/meditation", (req, res) => {
        if (!req.session.userId) return res.redirect("/signin")
        res.sendFile(path.join(__dirname, "public/html/meditation.html"))
    })

        app.get("/challenge", (req, res) => {
        if (!req.session.userId) return res.redirect("/signin")
        res.sendFile(path.join(__dirname, "public/html/challenge.html"))
    })

    app.get("/goals", (req, res) => {
        if (!req.session.userId) return res.redirect("/signin")
        res.sendFile(path.join(__dirname, "public/html/goals.html"))
    })

    app.get("/rewards", (req, res) => {
        if (!req.session.userId) return res.redirect("/signin")
        res.sendFile(path.join(__dirname, "public/html/rewards.html"))
    })

    app.get("/settings", (req, res) => {
        if (!req.session.userId) return res.redirect("/signin")
        res.sendFile(path.join(__dirname, "public/html/settings.html"))
    })

    app.get("/help", (req, res) => {
        if (!req.session.userId) return res.redirect("/signin")
        res.sendFile(path.join(__dirname, "public/html/help&feedback.html"))
    })

    app.get("/exit", (req, res) => {
        req.session.destroy(err => {
            if (err) {
                return res.redirect('/menu');
            }
            res.clearCookie('connect.sid');
            res.redirect('/signin');
        });
    });


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
        const { email, password, securityQuestion, securityAnswer } = req.body
        
        if (!email || !password || !securityQuestion || !securityAnswer) {
            return res.status(400).json({ error: "Please fill all fields." })
        }

        try {
            const hashedPassword = await bcrypt.hash(password, 10)
            const storedAnswer = securityAnswer.trim().toLowerCase()

            const result = await dbRun(
                `INSERT INTO users (email, password, security_question, security_answer, display_name) VALUES (?, ?, ?, ?, ?)`,
                [email, hashedPassword, securityQuestion, storedAnswer, "User"]
            )
            
            await dbRun(`INSERT OR IGNORE INTO user_stats (user_id) VALUES (?)`, [result.lastID]);
            
            res.status(201).json({ success: true, message: "Account created successfully!" })

        } catch (error) {
            console.error("Signup error:", error.message)
            if (error.message.includes("UNIQUE constraint failed: users.email")) {
                return res.status(400).json({ error: "User already exists." })
            }
            res.status(500).json({ error: "Internal server error." })
        }
    })

    // signin 
    app.post("/signin", (req, res) => {
        const { email, password } = req.body
        db.get(`SELECT * FROM users WHERE email = ?`, [email], async (err, user) => {
            if (err || !user) {
                return res.status(401).json({ error: "User not found." })
            }
            const match = await bcrypt.compare(password, user.password)
            if (match) {
                req.session.userId = user.id
                await dbRun(`INSERT OR IGNORE INTO user_stats (user_id) VALUES (?)`, [user.id]);
                res.json({ success: true, redirectUrl: "/menu" })
            } else {
                res.status(401).json({ error: "Incorrect password." })
            }
        })
    })

    //get question
    app.get("/api/password-reset/question", async (req, res) => {
        const { email } = req.query;
        if (!email) {
            return res.status(400).json({ error: "Email is required." });
        }
        try {
            const user = await dbGet(`SELECT security_question FROM users WHERE email = ?`, [email]);
            if (!user || !user.security_question) {
                return res.status(404).json({ error: "User not found or no security question set." });
            }
            res.json({ securityQuestion: user.security_question });
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Server error." });
        }
    });

    //reset password API
    app.post("/api/password-reset/reset", async (req, res) => {
        const { email, securityAnswer, newPassword } = req.body;

        if (!email || !securityAnswer || !newPassword) {
            return res.status(400).json({ error: "All fields are required." });
        }

        try {
            const user = await dbGet(`SELECT security_answer FROM users WHERE email = ?`, [email]);
            if (!user) {
                return res.status(404).json({ error: "User not found." });
            }
            if (securityAnswer.trim().toLowerCase() !== user.security_answer) {
                return res.status(401).json({ error: "Incorrect answer to security question." });
            }
            
            const hashedPassword = await bcrypt.hash(newPassword, 10);
            await dbRun(
                `UPDATE users SET password = ? WHERE email = ?`,
                [hashedPassword, email]
            );
            
            res.json({ success: true, message: "Password reset successfully." });

        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Server error." });
        }
    });



    // COMMUNITY API's

    const ensureAuth = async (req, res, next) => {
        if (req.session.userId) {
            try {
                // Check if the user in the session still exists in the DB
                const user = await dbGet('SELECT id FROM users WHERE id = ?', [req.session.userId]);
                if (user) {
                    next(); // User exists, proceed
                } else {
                    // This is a "ghost session" for a user that was deleted.
                    req.session.destroy(err => {
                        res.clearCookie('connect.sid');
                        res.status(401).json({ error: "Unauthorized" });
                    });
                }
            } catch (err) {
                console.error("ensureAuth error:", err);
                res.status(500).json({ error: "Server error during auth check" });
            }
        } else {
            res.status(401).json({ error: "Unauthorized" });
        }
    };

    app.get("/api/posts", ensureAuth, async (req, res) => {
        try {
            const posts = await dbAll(`
                SELECT 
                    p.id, p.content, p.image_url, strftime('%Y-%m-%dT%H:%M:%SZ', p.created_at) AS created_at,
                    u.id as author_id, u.display_name as author_name, u.profile_image as author_image
                FROM posts p
                JOIN users u ON p.user_id = u.id
                ORDER BY p.created_at DESC
            `)

            const enrichedPosts = await Promise.all(posts.map(async (post) => {
            const comments = await dbAll(`
                    SELECT 
                        c.id, c.content, strftime('%Y-%m-%dT%H:%M:%SZ', c.created_at) AS created_at,
                        u.id as author_id, u.display_name as author_name, u.profile_image as author_image
                    FROM comments c
                    JOIN users u ON c.user_id = u.id
                    WHERE c.post_id = ?
                    ORDER BY c.created_at ASC
                `, [post.id])
                const { likeCount } = await dbGet(`SELECT COUNT(*) as likeCount FROM likes WHERE post_id = ?`, [post.id]);
                const userLiked = await dbGet(`SELECT 1 FROM likes WHERE post_id = ? AND user_id = ?`, [post.id, req.session.userId]);

                return { ...post, comments, likes: likeCount, liked: !!userLiked }
            }))

            res.json(enrichedPosts)

        } catch (err) {
            console.error(err)
            res.status(500).json({ error: "Failed to fetch posts" })
        }
    });

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

    app.post("/api/posts/:id/like", ensureAuth, async (req, res) => {
        const postId = req.params.id;
        const userId = req.session.userId;

        try {
            const liked = await dbGet(`SELECT * FROM likes WHERE user_id = ? AND post_id = ?`, [userId, postId]);
            if (liked) {
                await dbRun(`DELETE FROM likes WHERE user_id = ? AND post_id = ?`, [userId, postId]);
                res.json({ success: true, liked: false });
            } else {
                await dbRun(`INSERT INTO likes (user_id, post_id) VALUES (?, ?)`, [userId, postId]);
                res.json({ success: true, liked: true });
            }
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to toggle like" });
        }
    });

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

    app.delete("/api/posts/:id", ensureAuth, async (req, res) => {
        const postId = req.params.id;
        const userId = req.session.userId;

        try {
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

    app.delete("/api/comments/:id", ensureAuth, async (req, res) => {
        const commentId = req.params.id;
        const userId = req.session.userId;

        try {
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

    app.get("/api/tasks", ensureAuth, async (req, res) => {
        const userId = req.session.userId;
        const category = req.query.category;

        let categoryWhere = " ";
        let params = [userId];

        if (category && category !== 'all') {
            categoryWhere = " AND category = ? ";
            params.push(category);
        }

        try {
            const tasks = await dbAll(`
                SELECT * FROM tasks 
                WHERE user_id = ? 
                ${categoryWhere} 
                AND (is_completed = 0 OR date(completed_at) = date('now'))
                ORDER BY
                    is_completed ASC,
                    CASE priority
                        WHEN 'high' THEN 1
                        WHEN 'medium' THEN 2
                        WHEN 'low' THEN 3
                        ELSE 4
                    END ASC,
                    created_at ASC
            `, params);
            res.json(tasks);
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to fetch tasks" });
        }
    });

    app.post("/api/tasks", ensureAuth, async (req, res) => {
        const { name, description, category, priority, frequency, customDays } = req.body;
        const userId = req.session.userId;

        if (!name) {
            return res.status(400).json({ error: "Task name is required" });
        }

        try {
            const result = await dbRun(
                `INSERT INTO tasks (user_id, name, description, category, priority, frequency, custom_days) 
                 VALUES (?, ?, ?, ?, ?, ?, ?)`,
                [userId, name, description, category, priority, frequency, customDays || null]
            );
            res.status(201).json({ success: true, taskId: result.lastID });
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to create task" });
        }
    });

    app.put("/api/tasks/:id/toggle", ensureAuth, async (req, res) => {
        const taskId = req.params.id;
        const userId = req.session.userId;

        try {
            const task = await dbGet(`SELECT * FROM tasks WHERE id = ? AND user_id = ?`, [taskId, userId]);
            if (!task) {
                return res.status(404).json({ error: "Task not found or unauthorized" });
            }

            const newStatus = !task.is_completed;
            const newCompletedAt = newStatus ? new Date().toISOString() : null;
            const today = new Date().toISOString().split('T')[0];

            if (newStatus === true) {
                // AWARDING POINTS
                const alreadyCompleted = await dbGet(
                    `SELECT * FROM daily_completions WHERE user_id = ? AND task_id = ? AND completed_date = ?`,
                    [userId, taskId, today]
                );
                
                if (!alreadyCompleted) {
                    // Check daily cap (max 4 tasks = 20 points)
                    const todayCount = await dbGet(
                        `SELECT COUNT(*) as count FROM daily_completions WHERE user_id = ? AND completed_date = ?`,
                        [userId, today]
                    );
                    
                    if (todayCount.count < 4) {
                        const streak = await calculateStreak(userId);
                        let multiplier = 1.0;
                        if (streak >= 30) multiplier = 2.5;
                        else if (streak >= 20) multiplier = 2.0;
                        else if (streak >= 10) multiplier = 1.5;
                        
                        const pointsToAward = 5 * multiplier;
                        
                        await dbRun(
                            `INSERT INTO daily_completions (user_id, task_id, completed_date, points_awarded) VALUES (?, ?, ?, ?)`,
                            [userId, taskId, today, pointsToAward]
                        );
                        
                        await dbRun(
                            `UPDATE user_stats SET total_points = total_points + ? WHERE user_id = ?`,
                            [pointsToAward, userId]
                        );
                        
                        await checkAndAwardAchievements(userId);
                    }
                }
            } else {
                // REMOVING POINTS
                const entry = await dbGet(
                    `SELECT * FROM daily_completions WHERE user_id = ? AND task_id = ? AND completed_date = ?`,
                    [userId, taskId, today]
                );
                
                if (entry) {
                    await dbRun(
                        `UPDATE user_stats SET total_points = total_points - ? WHERE user_id = ?`,
                        [entry.points_awarded, userId]
                    );
                    await dbRun(`DELETE FROM daily_completions WHERE id = ?`, [entry.id]);
                }
            }
            
            await dbRun(
                `UPDATE tasks SET is_completed = ?, completed_at = ? WHERE id = ?`,
                [newStatus, newCompletedAt, taskId]
            );
            
            res.json({ success: true, is_completed: newStatus });
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to toggle task" });
        }
    });


    app.get("/api/tasks/log", ensureAuth, async (req, res) => {
        const userId = req.session.userId;
        const category = req.query.category; 

        let query = `
            SELECT * FROM tasks 
            WHERE user_id = ? 
            AND is_completed = 1
        `;
        let params = [userId];

        if (category && category !== 'all') {
            query += ` AND category = ?`;
            params.push(category);
        }

        query += ` ORDER BY completed_at DESC`;

        try {
            const tasks = await dbAll(query, params);
            res.json(tasks);
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to fetch task log" });
        }
    });

    app.get("/api/statistics", ensureAuth, async (req, res) => {
        const userId = req.session.userId;
        const category = req.query.category;
        
        let categoryWhere = " ";
        let params = [userId];
        
        if (category && category !== 'all') {
            categoryWhere = " AND category = ? ";
            params.push(category);
        }

        try {
            const counts = await dbGet(`
                SELECT
                    COUNT(*) AS totalTasks,
                    SUM(CASE WHEN is_completed = 1 THEN 1 ELSE 0 END) AS tasksCompleted
                FROM tasks
                WHERE user_id = ? ${categoryWhere}
            `, params);

            const totalTasks = counts.totalTasks || 0;
            const tasksCompleted = counts.tasksCompleted || 0;
            
            const activeTasksResult = await dbGet(`
                SELECT COUNT(*) AS count
                FROM tasks
                WHERE user_id = ? ${categoryWhere} AND 
                (is_completed = 0 OR frequency IN ('daily', 'weekly', 'monthly', 'custom'))
            `, params);
            const activeTasks = activeTasksResult.count || 0;

            const completionRate = totalTasks === 0 ? 0 : Math.round((tasksCompleted / totalTasks) * 100);

            const trend = await dbAll(`
                SELECT 
                    strftime('%Y-%m-%d', completed_at) AS date,
                    COUNT(*) AS count
                FROM tasks
                WHERE user_id = ? ${categoryWhere} AND completed_at >= date('now', '-7 days')
                GROUP BY date
                ORDER BY date ASC
            `, params);

            const trendLabels = [];
            const trendData = [];
            for (let i = 6; i >= 0; i--) {
                const d = new Date();
                d.setDate(d.getDate() - i);
                const dateStr = d.toISOString().split('T')[0];
                trendLabels.push(dateStr);
                
                const dayData = trend.find(t => t.date === dateStr);
                trendData.push(dayData ? dayData.count : 0);
            }

            const categories = await dbAll(`
                SELECT category, COUNT(*) AS count
                FROM tasks
                WHERE user_id = ? ${categoryWhere}
                GROUP BY category
            `, params);

            const categoryLabels = categories.map(c => c.category);
            const categoryData = categories.map(c => c.count);

            const currentStreak = await calculateStreak(userId);
            
            res.json({
                currentStreak,
                completionRate,
                tasksCompleted: tasksCompleted,
                activeTasks: activeTasks,
                progressTrend: {
                    labels: trendLabels,
                    data: trendData
                },
                categoryBreakdown: {
                    labels: categoryLabels,
                    data: categoryData
                }
            });

        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to fetch statistics" });
        }
    });

    // --- API: User Progress for Level Bar ---
app.get("/api/user/progress", ensureAuth, async (req, res) => {
        const userId = req.session.userId;

        try {
            // 1. Get user's total points (Create stats row if missing)
            await dbRun(`INSERT OR IGNORE INTO user_stats (user_id) VALUES (?)`, [userId]);
            const stats = await dbGet(`SELECT total_points FROM user_stats WHERE user_id = ?`, [userId]);
            const totalPoints = stats ? stats.total_points : 0;

            // 2. Get all defined levels
            const allLevels = await dbAll(`SELECT * FROM achievements WHERE type = 'points' ORDER BY threshold ASC`);

            // --- FIX: If levels are missing, just return the points immediately ---
            if (allLevels.length === 0) {
                return res.json({
                    totalPoints: totalPoints, // <--- THIS SENDS YOUR 25 POINTS
                    currentLevel: { name: "Starter", icon: "🌱" },
                    nextLevel: null,
                    levels: [], 
                    progressPercentage: 0, 
                    pointsToNext: 0
                });
            }

            // 3. Normal Logic (Find user's current level)
            let currentLevel = allLevels[0];
            let nextLevel = allLevels[1];
            
            for (let i = allLevels.length - 1; i >= 0; i--) {
                if (totalPoints >= allLevels[i].threshold) {
                    currentLevel = allLevels[i];
                    nextLevel = allLevels[i + 1];
                    break;
                }
            }

            // 4. Calculate progress percentage
            let progressPercentage = 0;
            let pointsToNext = 0;
            let currentLevelPoints = currentLevel.threshold;
            let nextLevelPoints = nextLevel ? nextLevel.threshold : currentLevel.threshold;

            if (nextLevel) {
                const pointsInLevel = totalPoints - currentLevelPoints;
                const pointsForLevel = nextLevelPoints - currentLevelPoints;
                progressPercentage = (pointsForLevel > 0) ? (pointsInLevel / pointsForLevel) * 100 : 0;
                pointsToNext = nextLevelPoints - totalPoints;
            } else {
                progressPercentage = 100;
                pointsToNext = 0;
            }

            res.json({
                totalPoints: totalPoints,
                currentLevel: currentLevel,
                nextLevel: nextLevel,
                levels: allLevels, 
                progressPercentage: Math.min(progressPercentage, 100), 
                pointsToNext: pointsToNext
            });

        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to fetch user progress" });
        }
    });
    // --- END API ---

    app.delete("/api/tasks/:id", ensureAuth, async (req, res) => {
        const taskId = req.params.id;
        const userId = req.session.userId;

        try {
            const entries = await dbAll(
                `SELECT points_awarded FROM daily_completions WHERE user_id = ? AND task_id = ?`,
                [userId, taskId]
            );
            
            let totalPointsToRemove = 0;
            for (const entry of entries) {
                totalPointsToRemove += entry.points_awarded;
            }
            
            if (totalPointsToRemove > 0) {
                await dbRun(
                    `UPDATE user_stats SET total_points = total_points - ? WHERE user_id = ?`,
                    [totalPointsToRemove, userId]
                );
            }
            
            await dbRun(`DELETE FROM daily_completions WHERE user_id = ? AND task_id = ?`, [userId, taskId]);
            
            const result = await dbRun(
                `DELETE FROM tasks WHERE id = ? AND user_id = ?`,
                [taskId, userId]
            );

            if (result.changes === 0) {
                return res.status(404).json({ error: "Task not found or unauthorized" });
            }

            res.json({ success: true, message: "Task deleted" });
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to delete task" });
        }
    });

    //user API's
app.get("/api/user", (req, res) => {
    if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" })
    
    db.get("SELECT id, email, display_name, profile_image, banner_image, bio, created_at FROM users WHERE id = ?", [req.session.userId], (err, row) => {
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
        if (!req.session.userId) return res.status(401).json({ error: "Unauthorized" });
        
        // Get passwords from request body
        const { currentPassword, newPassword } = req.body;

        db.get("SELECT password FROM users WHERE id = ?", [req.session.userId], async (err, row) => {
            if (err) {
                console.error(err);
                return res.status(500).json({ error: "Database error" });
            }
            if (!row) {
                return res.status(404).json({ error: "User not found" });
            }
            
            const match = await bcrypt.compare(currentPassword, row.password);
            if (!match) {
                return res.status(401).json({ error: "Incorrect password" })
            }
            
            const newHashed = await bcrypt.hash(newPassword, 10)
            db.run("UPDATE users SET password = ? WHERE id = ?", [newHashed, req.session.userId], (err) => {
                if (err) {
                    console.error(err);
                    return res.status(500).json({ error: "Failed to update password" });
                }
                res.json({ success: true })
            })
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

app.get("/api/user/:id", (req, res) => {
    const userId = req.params.id;
    // Add 'created_at' to the SELECT statement
    db.get("SELECT id, display_name, profile_image, banner_image, bio, created_at FROM users WHERE id = ?", [userId], (err, row) => {
        if (err) return res.status(500).json({ error: "Database error" })
        if (!row) return res.status(404).json({ error: "User not found" })
        res.json(row)
    })
})


    // --- ACHIEVEMENT API's ---

    // Get displayed achievements for a specific user's profile
    app.get("/api/user/:id/achievements", async (req, res) => {
        try {
            const userId = req.params.id;
            const achievements = await dbAll(`
                SELECT a.name, a.description, a.icon 
                FROM user_achievements ua
                JOIN achievements a ON ua.achievement_id = a.id
                WHERE ua.user_id = ? AND ua.is_displayed = 1
                ORDER BY a.threshold ASC
            `, [userId]);
            res.json(achievements);
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to fetch achievements" });
        }
    });

    // Get ALL achievements (locked/unlocked) for the logged-in user to manage
    app.get("/api/user/achievements/manage", ensureAuth, async (req, res) => {
        try {
            const achievements = await dbAll(`
                SELECT 
                    a.id, a.name, a.description, a.icon,
                    (ua.user_id IS NOT NULL) as unlocked,
                    ua.is_displayed
                FROM achievements a
                LEFT JOIN user_achievements ua ON a.id = ua.achievement_id AND ua.user_id = ?
                ORDER BY a.threshold ASC
            `, [req.session.userId]);
            res.json(achievements);
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to fetch achievements" });
        }
    });

    // Toggle an achievement's display status
    app.put("/api/user/achievements/display", ensureAuth, async (req, res) => {
        try {
            const { achievement_id, display } = req.body;
            const userId = req.session.userId;
            
            const result = await dbRun(
                `UPDATE user_achievements SET is_displayed = ? WHERE user_id = ? AND achievement_id = ?`,
                [display ? 1 : 0, userId, achievement_id]
            );
            
            if (result.changes === 0) {
                return res.status(404).json({ error: "Achievement not unlocked by user" });
            }
            
            res.json({ success: true });
        } catch (err) {
            console.error(err);
            res.status(500).json({ error: "Failed to update achievement" });
        }
    });

    // --- TEMPORARY CHEAT CODE TO FIX POINTS ---
    app.get("/api/fix-points", ensureAuth, async (req, res) => {
        const userId = req.session.userId;
        console.log(`🛠️ FIXING POINTS FOR USER ID: ${userId}`);

        // 1. Ensure the stats row exists
        await dbRun(`INSERT OR IGNORE INTO user_stats (user_id) VALUES (?)`, [userId]);

        // 2. Force update the points to 25
        db.run(`UPDATE user_stats SET total_points = 25 WHERE user_id = ?`, [userId], (err) => {
            if (err) {
                console.error("❌ Error updating points:", err);
                return res.send("Error updating points: " + err.message);
            }
            console.log("✅ SUCCESS! Points forced to 25.");
            res.send(`
                <h1>✅ Success!</h1>
                <p>Points for User ID <b>${userId}</b> have been set to <b>25</b>.</p>
                <p><a href="/rewards">Click here to go back to Rewards page</a></p>
            `);
        });
    });
    // ------------------------------------------

    app.listen(PORT, () => console.log(`🚀 Server running at http://localhost:${PORT}`))

}

