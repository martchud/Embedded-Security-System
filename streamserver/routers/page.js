const express = require('express');
const router = express.Router();
const path = require('path');
const filePath = "/../public/";
router.get('/', function(req, res) {
    res.sendFile(path.join(__dirname + filePath + "homepage.html"));
});
router.get('/script.js', function(req, res) {
    res.sendFile(path.join(__dirname + filePath + "script.js"));
});
router.get('/style.css', function(req, res) {
    res.sendFile(path.join(__dirname + filePath + "stylesheets/style.css"));
});
module.exports = router;
