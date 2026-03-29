    const express = require("express");
    const cors = require("cors");
    const { exec } = require("child_process");
    const { GoogleGenerativeAI } = require("@google/generative-ai");

    const genAI = new GoogleGenerativeAI("AIzaSyAu07xn1LKS5CJhcvT5zHlv4wCsuevUB-w");

    const model = genAI.getGenerativeModel({
    model: "gemini-2.5-flash" // ← recommended
    });
    const fs = require("fs");
    const path = require("path");

    const app = express();
    app.use(express.static(path.join(__dirname, "..", "frontend")));
    app.use(cors());
    app.use(express.json());

    app.post("/compile", async (req, res) => {

        const code = req.body.code;

        const tempFilePath = path.join(__dirname, "temp.txt");
        fs.writeFileSync(tempFilePath, code);

        const compilerPath = path.join(__dirname, "..", "body", "compiler.exe");

       // ✅ Correct
exec(`cmd /c ""${compilerPath}" < "${tempFilePath}""`, async (error, stdout, stderr) => {

    const isError = stdout.includes("Type:") && stdout.includes("Error:");
    // ↑ your compiler always prints "Type:" and "Error:" for errors

    let compilerOutput = stdout || stderr || "";
    let aiExplanation = "";

    if (isError) {
        // Only call AI if there's actually an error
        try {
            const prompt = `
You are an AI programming tutor.

A student wrote this code:
${code}

The compiler produced this error:
${compilerOutput}

Explain the error in simple terms and suggest how to fix it.
`;
            const result = await model.generateContent(prompt);
            aiExplanation = result.response.text();

        } catch (err) {
            console.error("AI ERROR:", err);
            aiExplanation = "AI explanation unavailable.";
        }
    }

    res.json({
        success: !isError,           // 👈 tell frontend if it succeeded
        compiler_output: compilerOutput,
        ai_explanation: aiExplanation
    });
});

    });

    app.listen(5001, () => {
        console.log("Server running on http://localhost:5001");
    });


    // Add this line — serve the frontend folder
