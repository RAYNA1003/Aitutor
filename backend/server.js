const express = require("express");
const cors = require("cors");
const { exec } = require("child_process");
const fs = require("fs");
const path = require("path");

const app = express();
app.use(cors());
app.use(express.json());

app.post("/compile", (req, res) => {
    const code = req.body.code;

    // Save code to temporary file
    const tempFilePath = path.join(__dirname, "temp.txt");
    fs.writeFileSync(tempFilePath, code);

    // Path to your compiler.exe (adjust if needed)
    const compilerPath = path.join(__dirname, "..", "body", "compiler.exe");

    exec(`type "${tempFilePath}" | "${compilerPath}"`, (error, stdout, stderr) => {
        if (error) {
            return res.json({ output: stderr });
        }
        res.json({ output: stdout });
    });
});

app.listen(5000, () => {
    console.log("Server running on http://localhost:5000");
});