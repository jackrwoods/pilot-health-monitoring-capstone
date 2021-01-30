import { React, Component } from 'react';
import { w3cwebsocket as W3CWebSocket } from "websocket";
import './App.css';

// App frontend components
import BigNumber from './components/BigNumber'
import Graph from './components/Graph'
import Row from './components/Row'
import WarningPane from './components/WarningPane';

class App extends Component {

  // This constructor just instantiates constants for the websocket connection
  constructor(props) {
    super(props);

    this.wsClient = new W3CWebSocket('ws://192.168.1.1:8080');
  }

  componentWillMount() {
    this.wsClient.onopen = () => {
      console.log('WebSocket Client Connected');
    };

    this.wsClient.onmessage = (message) => {
      console.log(message);
    };
  }

  render() {
    return (
      <div className="App">
        {/* Warning pane */}
        <Row>
          <WarningPane>UNSTR TRND ▲ — ▼</WarningPane>
        </Row>
        <Row>
          <WarningPane>CON FAIL SRV FAIL H LAT</WarningPane>
        </Row>

        {/* Graphs and data visualization */}
        <Row>
          <Graph />
        </Row>
        <Row>
          <BigNumber uom="❤">
            84
          </BigNumber>
        </Row>
        <Row>
          <Graph />
        </Row>
        <Row>
          <BigNumber uom="%">
              96
          </BigNumber>
        </Row>
      </div>
    );
  }
}

export default App;
