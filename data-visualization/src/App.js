import { React, Component } from 'react';
import { w3cwebsocket as W3CWebSocket } from "websocket";
import './App.css';

// App frontend components
import BigNumber from './components/BigNumber'
import ConnectionStatus from './components/ConnectionStatus'
import Graph from './components/Graph'
import Row from './components/Row'
import Trend from './components/Trend'
import WarningPane from './components/WarningPane';

class App extends Component {

  // This constructor just instantiates constants for the websocket connection
  constructor(props) {
    super(props);

    // Establish ping and data connections
    this.pingClient = new W3CWebSocket('ws://192.168.1.15:8080/ping');
    this.wsClient = new W3CWebSocket('ws://192.168.1.15:8080/data');

    this.state = {
      connection: {
        connection: false,
        server: false,
      },
      data: {
        rawData: [],
        HR: [],
        avgHR: 70,
        SpO2: [],
        avgSpO2: 95
      },
      // Calculate ping
      ping: {
        connectionPingMillis: [], // Array of ping observations
        avgConnectionPing: 0, // The average ping
        pingEpoch: Date.now(), // Temporarily stores the "sent" time when calculating round-trip time
      }
    }
  }

  componentWillMount() {
    this.wsClient.onopen = () => {
      console.log('Data WebSocket Client Connected');
      this.setState((state) => {
        state.connection.connection = true
        state.connection.server = true
      })
    };

    this.wsClient.onclose = () => {
      console.log('Data WebSocket Client Disconnected');
      this.setState((state) => {
        state.connection.connection = false
        state.connection.server = false
      })
    }

    this.wsClient.onmessage = (message) => {
      this.setState((state => {
        let data = JSON.parse(message.data);
        state.data.rawData.push(data);
        state.data.HR.push(data.HR);
        state.data.SpO2.push(data.SpO2);
        if (state.data.HR.length > 2) {
          state.data.avgHR = (state.data.HR.reduce((a, b) => a + b) / state.data.HR.length).toFixed(1);
          state.data.avgSpO2 = (state.data.SpO2.reduce((a, b) => a + b) / state.data.SpO2.length).toFixed(1);
        }
        if (state.data.HR.length > 1000) { // Sensor reads at 100hz, so this is approximately 1 minute of data
          state.data.HR.shift(); // Delete the oldest point
          state.data.SpO2.shift();
          state.data.rawData.shift();

        }
      }));
    };

    this.pingClient.onopen = () => {
      console.log('Ping WebSocket Client Connected');

      this.interval = setInterval(() => {
        this.setState((state) => state.ping.pingEpoch = Date.now());
        this.pingClient.send(':)');
      }, 200);
    }

    this.pingClient.onmessage = (message) => {
      const now = Date.now();
      this.setState((state) => {
        state.ping.connectionPingMillis.push((now - state.ping.pingEpoch) / 2);
        if (state.ping.connectionPingMillis.length > 30) state.ping.connectionPingMillis.shift();
        state.ping.avgConnectionPing = Math.round(state.ping.connectionPingMillis.reduce((a, b) => a + b) / state.ping.connectionPingMillis.length);
      });
    }
  }

  render() {
    return (
      <div className="App">
        {/* Warning pane */}
        <Row>
          <WarningPane><Trend data={this.state.data} /></WarningPane>
        </Row>
        <Row>
          <WarningPane><ConnectionStatus connection={this.state.connection} ping={this.state.ping.avgConnectionPing} /></WarningPane>
        </Row>

        {/* Graphs and data visualization */}
        <Row>
          <Graph getData={() => ({data: this.state.data.HR, avg: this.state.data.avgHR})}/>
        </Row>
        <Row>
          <BigNumber uom="❤">
            {this.state.data.HR.length > 2 ? this.state.data.avgHR : 'Loading '}
          </BigNumber>
        </Row>
        <Row>
          <Graph getData={() => ({data: this.state.data.SpO2, avg: this.state.data.avgSpO2})}/>
        </Row>
        <Row>
          <BigNumber uom="%">
          {this.state.data.SpO2.length > 2 ? this.state.data.avgSpO2 : 'Loading '}
          </BigNumber>
        </Row>
      </div>
    );
  }
}

export default App;
