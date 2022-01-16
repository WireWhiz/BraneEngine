const pageChanged = new Event('pageChanged');
function currentPage(){
    return window.location.pathname.substr(5);
}
function changePage(page){
    if(page != currentPage()){
        window.history.pushState({}, null, "/app/" + page);
        document.dispatchEvent(pageChanged);
    }

}

class SidebarButton extends  React.Component{
    constructor(props) {
        super(props);
    }

    render() {
        return (
            <button class={"sidebar-button"} onClick={()=>{changePage(this.props.page)}}>
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
        this.state = {
            page: currentPage()
        };

        document.addEventListener('pageChanged', (e)=> {
            this.setState({
                page: currentPage()
            });
        });

        window.onpopstate = (e)=>{
            this.setState({
                page:currentPage()
            });
        }
    }
    renderContent(){
        switch(this.state.page){
            case "profile":
                return <Profile/>
            case "assets":
                return <Assets/>
            default:
                changePage("profile")
        }
    };

    render() {
        return (
            <div class={"content"}>
                {this.renderContent()}
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

