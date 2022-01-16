document.getElementById("email").addEventListener('keyup', function(evt){
    if(evt.key === "Enter" || evt.keyCode === 13)
        document.getElementById("uname").focus();
});
document.getElementById("uname").addEventListener('keyup', function(evt){
    if(evt.key === "Enter" || evt.keyCode === 13)
        document.getElementById("password").focus();
});
document.getElementById("password").addEventListener('keyup', function(evt){
    if(evt.key === "Enter" || evt.keyCode === 13)
        document.getElementById("create-account-submit").click();
});

function createAccount(){
    var username = document.getElementById("uname").value;
    var password = document.getElementById("password").value;
    var email = document.getElementById("email").value;

    if(username.length == 0)
    {
        document.getElementById("login-result").innerHTML = "Must enter username";
        return;
    }

    if(!email.match(/^([a-zA-Z0-9_\-\.]+)@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.)|(([a-zA-Z0-9\-]+\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\]?)$/))
    {
        document.getElementById("login-result").innerHTML = "Invalid Email!";
        return;
    }

    if(password.length < 8)
    {
        document.getElementById("login-result").innerHTML = "Password must be at least 8 characters long";
        return;
    }

    let login_request = new Request("/create-account-submit", {
        method: "POST",
        mode: "same-origin",
        cache: "no-cache",
        body: JSON.stringify({
            username : username,
            password : password,
            email : email
        })
    })
    fetch(login_request).then(function(res){
        return res.json();
    }).then(function(json){
        document.getElementById("login-result").innerHTML = json.text;
        if(json.created)
            window.location.href = "/login"
    });

    /*var loginRequest = new XMLHttpRequest();
    loginRequest.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            // Typical action to be performed when the document is ready:
            document.getElementById("login-result").innerHTML = loginRequest.responseText;
        }
    };
    loginRequest.open("POST", "create-account-submit", true);
    loginRequest.setRequestHeader("username", username);
    loginRequest.setRequestHeader("password", password);
    loginRequest.setRequestHeader("email", email);
    loginRequest.send();*/
}