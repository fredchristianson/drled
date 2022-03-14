export function configure(env) {
    console.log("env-dev");

    env.DEBUG = true;
    env.PROD = false;

    env.LED_STRIPS = [
        {name: 'Living Room',host:"lr.400ggp.com"},
        {name: 'Dining Room',host:"dr.400ggp.com"},
        {name: 'Kitchen Cupboard',host:"kc.400ggp.com"},
        {name: 'Kitchen Floor',host:"kf.400ggp.com"},
        {name: 'Lanai',host:"lanai.400ggp.com"},
        {name: 'Hall',host:"hall.400ggp.com"},
        {name: 'Test',host:"192.168.10.187"},
        {name: 'Demo',host:"192.168.10.130"},
        {name: 'Circle',host:"192.168.10.132"}
    ];
}