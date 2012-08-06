
/*
 * main.c
 *
 *  Created on: 11.06.2010
 *      Author: tobi
 *  Modified on 07.03.2012
 *      Author: Alexander Barth
 */


unsigned char i2c_buffer[4] = {0, 0, 0, 0};
static periodic_thread_t motors_thread;
#define RT_MOTORS_PERIOD (RT_THREAD_PERIOD * 3)


PERIODIC_THREAD_BEGIN(motors_thread_func)
{
   PERIODIC_THREAD_LOOP_BEGIN
   {
#if MOTORS_ENABLED
      motors_write_uint8(i2c_buffer);
#endif
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END




static udp_socket_t *udp_socket = NULL;

static periodic_thread_t realtime_thread;
#define RT_THREAD_PERIOD 3
PERIODIC_THREAD_BEGIN(realtime_thread_func)
{ 
   FILE *fp = fopen("/root/ARCADE_UAV/components/core/temp/log.dat","w");
   if (fp == NULL) printf("ERROR: could not open File");
   fprintf(fp,"gyro_x gyro_y gyro_z motor1 motor2 motor3 motor4 battery_voltage rc_input0 rc_input1 rc_input2 rc_input3 u_ctrl1 u_ctrl2 u_ctrl3\n");

   /*threadsafe_float_t speed_p;
   threadsafe_float_t speed_i;
   threadsafe_float_t speed_i_max;
   
   threadsafe_float_t pos_p;
   threadsafe_float_t pos_i;
   threadsafe_float_t pos_i_max;*/
   //ControllerData controller;
   //ControllerNew controller;
   PIIDController controller;
   Filter2nd filter_out;
   filter_lp_init(&filter_out,55.0f,0.95f,0.003f,4);

   
   piid_controller_init(&controller,0.003f);
   //new_controller_init(&controller,0.003f);
   controller.f_local[0] = 1.50f;

   long timer_counter = 0;

   struct timespec ts_curr;
   struct timespec ts_prev;
   struct timespec ts_diff;
   UdpData udp_data_out;
   clock_gettime(CLOCK_REALTIME, &ts_curr);

   PERIODIC_THREAD_LOOP_BEGIN
   {
      /*
       * calculate dt:
       */
      ts_prev = ts_curr;
      clock_gettime(CLOCK_REALTIME, &ts_curr);
      TIMESPEC_SUB(ts_diff, ts_curr, ts_prev);
      float _dt = (float)ts_diff.tv_sec + (float)ts_diff.tv_nsec / (float)NSEC_PER_SEC;

      /*
       * read sensor values into model input structure:
       */
      model_input_t model_input;
      float dt = (float)RT_THREAD_PERIOD / 1000.0f;
      model_input.dt = dt;
      controller.Ts = dt;

      ahrs_read(&model_input.ahrs_data);
      gps_read(&model_input.gps_data);
      //model_input.ultra_z = ultra_altimeter_read();
      //model_input.baro_z = baro_altimeter_read();
      
      mixer_in_t mixer_in;
#if 1
      /*
       * execute model step:
       */
      model_state_t model_state;
      model_step(&model_state, &model_input);

      float dir = model_input.gps_data.course; //atan2(model_state.y.speed, model_state.x.speed);
      char buf[100];
      sprintf(buf, "%f,%f;%f,%f", model_state.x.speed, model_state.y.speed, cos(dir) * model_input.gps_data.speed, sin(dir) * model_input.gps_data.speed);
      EVERY_N_TIMES(100, udp_socket_send(udp_socket, buf, strlen(buf)));

      /*
       * execute controller step:
       */
      ctrl_step(&mixer_in, dt, &model_state);
#endif
      /*
       * write data to mixer at OUTPUT_RATIO:
       */
      
      /* determine attitude */
      float gyro_vals[3];
      float u_ctrl[3];
      float rc_input[4] = {0.0f,0.0f,0.0f, 0.0f};
      gyro_vals[0] = -model_input.ahrs_data.gyro_x;
      gyro_vals[1] = model_input.ahrs_data.gyro_y;
      gyro_vals[2] = -model_input.ahrs_data.gyro_z;

      udp_data_out.angle[0] = model_input.ahrs_data.roll; 
      udp_data_out.angle[1] = -model_input.ahrs_data.pitch;
      udp_data_out.angle[2] = model_input.ahrs_data.yaw;

      //EVERY_N_TIMES(100,printf("Angles: %5.3f %5.3f %5.3f\n",model_input.ahrs_data.roll,model_input.ahrs_data.pitch,model_input.ahrs_data.yaw));
      
      /* get (and scale) values from remote interface */
      float rc_pitch = (float)rc_dsl_reader_get_channel(0) / 2000.0f-rc_bias[0];
      if (fabs(rc_pitch)<0.005f) rc_pitch = 0.0f;

      float rc_roll = (float)rc_dsl_reader_get_channel(1) / 2000.0f-rc_bias[1];
      if (fabs(rc_roll)<0.005f) rc_roll = 0.0f;

      float rc_yaw = -(float)rc_dsl_reader_get_channel(3) / 2000.0f-rc_bias[2];
      if (fabs(rc_yaw)<0.005f) rc_yaw = 0.0f;

      float rc_gas = (float)rc_dsl_reader_get_channel(2) / 4000.0f + 0.5f;
      float rc_ch5 = (float)rc_dsl_reader_get_channel(4) / 2000.0f;

      rc_input[0] = -2.0f * rc_roll + 0.1 * mixer_in.roll; // 0.1
      rc_input[1] = 2.0f * rc_pitch - 0.1 * mixer_in.pitch; // 0.1
      rc_input[2] = -3.0f * rc_yaw + 0.0 * mixer_in.yaw;
      controller.f_local[0] = 30.0f * rc_gas;

      piid_controller_run(&controller,gyro_vals,rc_input,u_ctrl);
      filter_lp_run(&filter_out,controller.f_local,controller.f_local);
      controller.int_enable = force2twi(controller.f_local, &voltage,i2c_buffer);
      fprintf(fp,"%10.9f %10.9f %10.9f %d %d %d %d %6.4f %6.4f %6.4f %6.4f %6.4f %10.9f %10.9f %10.9f\n",gyro_vals[0],gyro_vals[1],gyro_vals[2],i2c_buffer[0],i2c_buffer[1],i2c_buffer[2],i2c_buffer[3],voltage,controller.f_local[0],rc_input[0], rc_input[1], rc_input[2], u_ctrl[0], u_ctrl[1], u_ctrl[2]);
      
      udp_data_out.motors[0] = i2c_buffer[0];
      udp_data_out.motors[1] = i2c_buffer[1];
      udp_data_out.motors[2] = i2c_buffer[2];
      udp_data_out.motors[3] = i2c_buffer[3];
      
      if (rc_ch5 > 0.0 || !rc_dsl_reader_signal_valid())
      {
         controller.int_enable = 0;
         i2c_buffer[0] = 0;
         i2c_buffer[1] = 0;
         i2c_buffer[2] = 0;
         i2c_buffer[3] = 0;
      }


      EVERY_N_TIMES(20,udp_params_send(&udp_data_out));

   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END



void _main(int argc, char *argv[])
{
   syslog(LOG_INFO, "initializing core");
   if (zmq_ipc_init("core") != 0)
   {
      syslog(LOG_CRIT, "could not init zmq_ipc module");
      exit(EXIT_FAILURE);
   }
   params_init();

   syslog(LOG_CRIT, "%d %s", argc, argv[2]);
   handle_cmd_line(argc, argv);
   syslog(LOG_CRIT, "opening logger");

   if (logger_open() != 0)
   {
      syslog(LOG_CRIT, "could not open logger");
      exit(EXIT_FAILURE);
   }
   sleep(1);

   LOG(LL_INFO, "+------------------+");
   LOG(LL_INFO, "|   core startup   |");
   LOG(LL_INFO, "+------------------+");

   LOG(LL_INFO, "initializing system");

   struct sched_param sp;
   sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
   sched_setscheduler(getpid(), SCHED_FIFO, &sp);

   if (mlockall(MCL_CURRENT | MCL_FUTURE))
   {
      LOG(LL_ERROR, "mlockall() failed");
   }

   rc_dsl_reader_start();
   omap_i2c_bus_init();
   usb_i2c_bus_init();
   
   //baro_altimeter_init();
   //ultra_altimeter_init();
   ahrs_init();
   /*leds_overo_initialize();*/
   gps_init();
   motors_init();

   udp_params_init();

   LOG(LL_INFO, "initializing model/controller");
   model_init();
   ctrl_init();
   
   LOG(LL_INFO, "initializing cmd/params interface");
   cmd_init();
   params_thread_start();
   
   printf("Press 's' and return to continue\n");
   char start_key = 0;
   while (start_key != 's')
   {
      start_key = getchar();
   }
   const char temp_mot[4] = {20, 20, 20, 20};
   unsigned int timer = 0;
   for (timer = 0; timer<400; timer++)
   {
      rc_bias[0] += (float)rc_dsl_reader_get_channel(0) / 2000.0f;
      rc_bias[1] += (float)rc_dsl_reader_get_channel(1) / 2000.0f;
      rc_bias[2] += -(float)rc_dsl_reader_get_channel(3) / 2000.0f;

      usleep(12000);
      if (rc_dsl_reader_signal_valid())
      {
#if MOTORS_ENABLED
         motors_write_uint8(temp_mot);
#endif
      }
   }
   rc_bias[0] = rc_bias[0]/400;
   rc_bias[1] = rc_bias[1]/400;
   rc_bias[2] = rc_bias[2]/400;
   for(int n = 0; n < NUM_AVG; n++)
   {
      output_avg[n] = sla_create(OUTPUT_RATIO, 0.0f);
   }
   LOG(LL_INFO, "system up and running");
   const struct timespec period = {0, RT_THREAD_PERIOD * NSEC_PER_MSEC};
   const struct timespec motors_period = {0, RT_MOTORS_PERIOD * NSEC_PER_MSEC};
   const struct timespec battery_period = {BATTERY_THREAD_PERIOD_S, 0};
   
   udp_socket = udp_socket_create("10.0.0.7", 1337, 0, 0);
   periodic_thread_start(&realtime_thread, realtime_thread_func, "realtime_thread",
                         99, period, NULL);
   
   periodic_thread_start(&battery_thread, battery_thread_func, "battery_thread",
                         0, battery_period, NULL);
   
   periodic_thread_start(&motors_thread, motors_thread_func, "motors_thread",
                         99, motors_period, NULL);


   while (1)
   {
      sleep(1);
   }
}


void _cleanup(void)
{
   static int killing = 0;
   if (!killing)
   {
      killing = 1;
      LOG(LL_INFO, "system shutdown by user");
      sleep(1);
      kill(0, 9);
   }
}

