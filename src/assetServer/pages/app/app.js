const pageChanged = new Event('pageChanged');

function currentPath(){
    return window.location.pathname.substr(5).split("/");
}
function changePath(path){

    if(path != window.location.pathname.substr(5)){
        window.history.pushState({}, null, "/app/" + path);
        document.dispatchEvent(pageChanged);
    }

}

class SidebarButton extends  React.Component{
    constructor(props) {
        super(props);
    }

    render() {
        return (
            <button class={"sidebar-button"} onClick={()=>{changePath(this.props.page)}}>
                {this.props.text}
            </button>
        );
    }
}

class Sidebar extends React.Component{
    constructor(props) {
        super(props);
    }

    render() {
        return (
            <div class={"sidebar"}>
                <SidebarButton text={<p><span className="material-icons-outlined">account_circle</span><br/>Profile</p>} page={"profile"} />
                <SidebarButton text={<p><span className="material-icons-outlined">dns</span><br/>Assets</p>} page={"assets"} />
            </div>
        );
    }
}

class Content extends React.Component{
    constructor(props) {
        super(props);

        document.addEventListener('pageChanged', (e)=> {
            this.forceUpdate();
        });

        window.onpopstate = (e)=>{
            this.forceUpdate();
        }
    }

    render() {
        return (
            <div class={"content"}>
                <PathBranch pageDepth={0} basePage={<Profile/>} pages={{
                    "profile" : <Profile/>,
                    "assets" : <Assets/>
                }}/>
            </div>
        );
    }
}

class App extends React.Component{
    constructor(props) {
        super(props);
    }

    render() {
        return (
            <div class={"app"}>
                <Sidebar />
                <Content />
            </div>
        );
    }
}

ReactDOM.render(
    <App />,
    document.getElementById('root')
);

