import React, { Component } from 'react';
import { Line } from 'react-chartjs-2';

class Graph extends Component {
  constructor(props) {
    super(props);
    this.chartRef = React.createRef();
    this.data = {
      datasets: [
        {
          label: 'Data',
          backgroundColor: [
            'rgb(255, 255, 255)'
          ],
          borderColor: [
            'rgb(255, 255, 255)'
          ],
          borderWidth: '4',
          data: Array(60).fill().map((el, i) => 15 * Math.sin(i) * Math.random() + 86),
          fill: 'false',
          pointRadius: '0px'
        }
      ],
      labels: Array(60).fill().map((el, i) => i - 60), // Spans from -60 seconds to present
    };
    this.options = {
      layout: {
        padding: {
          top: 20
        }
      },
      legend: {
        display: false
      },
      maintainAspectRatio: false,
      scales: {
        xAxes: [{
          gridLines: {
            color: 'rgb(255, 255, 255)',
            display: true,
            drawBorder: true,
            drawOnChartArea: false,
            lineWidth: '1'
          },
          ticks: {
            fontColor: 'rgb(255, 255, 255)',
            maxTicksLimit: 7,
            stepSize: 10,
            suggestedMax: 0, // AVG + 10
            suggestedMin: -60 // AVG - 10
          }
        }],
        yAxes: [{
          gridLines: {
            borderDash: [2],
            color: 'rgb(255, 255, 255)',
            lineWidth: '1'
          },
          ticks: {
            fontColor: 'rgb(255, 255, 255)',
            maxTicksLimit: 3,
            stepSize: 10,
            suggestedMax: 100, // AVG + 10
            suggestedMin: 1 // AVG - 10
          }
        }]
      }
    };
  }

  render() {
    return (
      <Line ref={this.chartRef} data={this.data} options={this.options} width={'100%'} height={ window.innerHeight * 0.3 + 'px' }/>
    )
  }
}



export default Graph;