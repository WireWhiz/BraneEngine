document.getElementById("uname").addEventListener('keyup', function(evt){
    if(evt.key === "Enter" || evt.keyCode === 13)
        document.getElementById("password").focus();
});
document.getElementById("password").addEventListener('keyup', function(evt){
    if(evt.key === "Enter" || evt.keyCode === 13)
        document.getElementById("login-submit").click();
});

function logIn(){
    var username = document.getElementById("uname").value;
    var password = document.getElementById("password").value;

    var loginRequest = new XMLHttpRequest();
    loginRequest.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            // Typical action to be performed when the document is ready:
            document.getElementById("login-result").innerHTML = loginRequest.responseText;
        }
    };
    loginRequest.open("POST", "login-submit", true);
    loginRequest.setRequestHeader("username", username);
    loginRequest.setRequestHeader("password", password);
    loginRequest.send();
}