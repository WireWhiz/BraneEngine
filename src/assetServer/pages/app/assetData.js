class AssetView extends React.Component{
    constructor(props) {
        super(props);
        this.state = {
            dataLoaded : false,
            loadMessage : "",
            asset : {}
        }
    }

    getAssetData(){
        fetch("/api/assets/" + this.props.assetID).then((res) =>{
            return res.json();
        }).then((json) =>{
            if(!json["successful"])
            {
                this.setState({
                    dataLoaded : true,
                    loadMessage : json["text"]
                });
                return;
            }

            //Stuff with asset here.

            this.setState({
                dataLoaded : true,
                asset : json["asset"],
                loadMessage : "Loaded"
            });
        });

    }

    render() {
        console.log("Viewing asset: " + this.props.assetID);
        if(!this.state.dataLoaded)
            this.getAssetData();

        let page = (this.state.asset != null) ? [
            <h4>View Asset: {this.state.asset.name}</h4>,
            <h4>AssetID: {this.props.assetID}</h4>

        ] : [
            <h1>Loading: {this.props.assetID}</h1>,
            <p>{this.state.loadMessage}</p>
        ];

        this.state.dataLoaded = false;

        return page;
    }
}