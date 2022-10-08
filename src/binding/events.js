import { EventEmitter2 } from 'eventemitter2'

const EventEmitter = EventEmitter2
EventEmitter.EventEmitter = EventEmitter

module.exports = EventEmitter
