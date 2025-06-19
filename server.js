// server.js - Node.js API to run the C++ toy compiler
const express = require('express');
const fs = require('fs');
const { exec } = require('child_process');
const path = require('path');
const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(express.json());
app.use(express.static(path.join(__dirname)));

// POST /run - receives code and runs toy_compiler
app.post('/run', (req, res) => {
  const code = req.body.code;
  if (!code) {
    return res.status(400).send('No code received');
  }

  // Save code to a temp input file
  const inputPath = path.join(__dirname, 'input.txt');
  fs.writeFile(inputPath, code, (err) => {
    if (err) return res.status(500).send('Error writing input file');

    // Run the compiled toy_compiler with redirected input
    exec(`./toy_compiler < ${inputPath}`, (err, stdout, stderr) => {
      if (err) {
        res.status(500).send(`Compiler error:\n${stderr}`);
      } else {
        res.send(stdout);
      }
    });
  });
});

// Start server
app.listen(PORT, () => {
  console.log(`✅ Server running at http://localhost:${PORT}`);
});
