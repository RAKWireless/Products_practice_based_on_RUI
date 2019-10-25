## Introduction

This demo shows how to use gpio interrupt by acc sensor.

1. lis3dh interrupt pin is 16 
2. When shake 5010, lis3dh will make 5010 sleep. Then shake, it will wake up. again and again
3. If use interrupt, the current will increase including lis3dh work power and gpio power of mcu.

