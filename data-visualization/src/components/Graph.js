import React, { Component } from 'react';
import WebGLplot, { WebglLine, ColorRGBA } from 'webgl-plot';
import './Graph.css';

class Graph extends Component {
  constructor(props) {
    super(props)
    this.chartRef = React.createRef();
    this.color = new ColorRGBA(255, 255, 255, 1);
  }

  componentDidMount() {
    // Initial setup
    const devicePixelRatio = window.devicePixelRatio || 1;
    this.chartCanvas = this.chartRef.current;
    this.chartCanvas.width = this.chartCanvas.clientWidth * devicePixelRatio;
    this.chartCanvas.height = this.chartCanvas.clientHeight * devicePixelRatio;
    this.line = new WebglLine(this.color, 1000);
    this.wglp = new WebGLplot(this.chartCanvas);

    this.line.lineSpaceX(-1, 2 / 1000);
    console.log(2 / 1000)
    this.wglp.addLine(this.line);
    setInterval(() => {
      const data = this.props.getData();
      for (let i = 0; i < 1000; i++) {
        this.line.setY(i, (data.data[i] -  data.avg) / 10);
      }
      this.wglp.update();
    }, 17)
  }

  render() {
    return (
      <div class="grid-container">
        <div class="canvas"><canvas ref={this.chartRef} style={{display: 'block', left: '0px', width: '100vw', height: window.innerHeight * 0.3 + 'px'}} id="my_canvas" /></div>
        <div class="topLabel">{this.props.getData().avg + 10}</div>
        <div class="bottomLabel">{this.props.getData().avg - 10}</div>
      </div>
          


    )
  }
}

export default Graph;