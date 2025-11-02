// server.js
const express = require('express');
const sqlite3 = require('sqlite3').verbose();
const bcrypt = require('bcryptjs');
const session = require('express-session');
const bodyParser = require('body-parser');
const path = require('path');

const app = express();
const PORT = 3000;

// Middleware
app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));
app.use(session({
  secret: 'microhabits_secret_key',
  resave: false,
  saveUninitialized: true
}));

// Database setup
const db = new sqlite3.Database('./database.db', (err) => {
  if (err) console.error('Error opening database:', err);
  else {
    db.run(`
      CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        email TEXT UNIQUE,
        password TEXT
      )
    `);
    console.log('✅ Database ready');
  }
});

// Serve HTML pages
app.get('/', (req, res) => res.sendFile(path.join(__dirname, 'public', 'signin.html')));
app.get('/signup', (req, res) => res.sendFile(path.join(__dirname, 'public', 'signup.html')));

// Sign Up route
app.post('/signup', async (req, res) => {
  const { email, password, confirmPassword } = req.body;

  if (password !== confirmPassword) {
    return res.send('<h3>❌ Passwords do not match.</h3><a href="/signup">Go Back</a>');
  }

  const hashedPassword = await bcrypt.hash(password, 10);
  db.run(`INSERT INTO users (email, password) VALUES (?, ?)`, [email, hashedPassword], (err) => {
    if (err) {
      if (err.message.includes('UNIQUE constraint')) {
        res.send('<h3>❌ Email already exists.</h3><a href="/signup">Try again</a>');
      } else {
        res.send('<h3>⚠️ Something went wrong.</h3>');
      }
    } else {
      res.send('<h3>✅ Sign up successful! <a href="/">Log In</a></h3>');
    }
  });
});

// Sign In route
app.post('/signin', (req, res) => {
  const { email, password } = req.body;

  db.get(`SELECT * FROM users WHERE email = ?`, [email], async (err, user) => {
    if (err) return res.send('⚠️ Database error');
    if (!user) return res.send('<h3>❌ User not found. <a href="/">Try again</a></h3>');

    const valid = await bcrypt.compare(password, user.password);
    if (valid) {
      req.session.user = user;
      res.send(`<h2>✅ Welcome ${user.email}!</h2><p>You’re logged in.</p>`);
    } else {
      res.send('<h3>❌ Incorrect password. <a href="/">Try again</a></h3>');
    }
  });
});

// Run server
app.listen(PORT, () => console.log(`🚀 Server running on http://localhost:${PORT}`));
