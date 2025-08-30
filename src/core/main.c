/* main.c - Application Entry Point */
#include "main.h"      // For Error_Handler() and HAL types
#include "app_logic.h" // For app_setup() and app_loop()

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  // Initialize and configure the system, peripherals, and application.
  app_setup();

  // The main loop of the application.
  while (1)
  {
    app_loop();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    // Stay in an infinite loop on error.
  }
}