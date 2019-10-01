#include "filetofiletask.h"

#include "../system/interactivefactory.h"

namespace twPro {

    static size_t OPTIMAL_NOTIFIER_QUEUE_SIZE = 5;

    FileToFileTask::FileToFileTask(const twPro::SourceToResultTemplateTask_params & _params) noexcept :
        SourceToResultTemplateTask(_params)
    {
    }


    FileToFileTask::~FileToFileTask() noexcept
    {
    }

    void FileToFileTask::preCheck(const twPro::SourceToResultTemplateTask_params & _params)
    {
        if (_params.blockSize < 1) {
            throw std::runtime_error("Block size can't be less than 1");
        }
        else if (_params.source.empty() || _params.result.empty()) {
            throw std::runtime_error("File path can't be empty");
        }
        else if (_params.source == _params.result) {
            throw std::runtime_error("Input and output files can't have similar path");
        }
    }

    void FileToFileTask::setupSources(std::shared_ptr<twPro::LRDataBuffer> _sourceBufferPtr, std::shared_ptr<twPro::LRDataBuffer> _resultBufferPtr)
    {
        // === SETUP RESOURCES  ===

        m_sourceFilePtr.reset(new twPro::FileReaderByParts(parameters().source));
        m_sourceFilePtr->setProducerBuffer(_sourceBufferPtr);

        m_resultFilePtr.reset(new twPro::FileWriterByParts(parameters().result));
        m_resultFilePtr->setConsumerBuffer(_resultBufferPtr);

        // === PRINT MAIN INFO ===

        B_INFO << "Task block size: " << parameters().blockSize << " bytes" << E_INFO;
        B_INFO << "Source data length: " << totalData() << " bytes" << E_INFO;
    }

    std::function<void()> FileToFileTask::prepareSourceJob(std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret)
    {
        auto fileReaderJob = [this, &_stopFlag, &_ret]() {

            try {
                m_sourceFilePtr->work(_stopFlag);
            }
            catch (const std::exception& ex) {
                _ret = false;
                _ret.error = ex.what();
            }
            catch (const std::string& ex) {
                _ret = false;
                _ret.error = ex;
            }
            catch (...) {
                _ret = false;
                _ret.error = "File reader returns unknown error";
            }

            if (!_ret) {
                _stopFlag = true;
            }

        };

        return fileReaderJob;
    }

    std::function<void()> FileToFileTask::prepareResultJob(std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret)
    {
        auto fileWriterJob = [this, &_stopFlag, &_ret]() {

            try {
                m_resultFilePtr->work(_stopFlag);
            }
            catch (const std::exception& ex) {
                _ret = false;
                _ret.error = ex.what();
            }
            catch (const std::string& ex) {
                _ret = false;
                _ret.error = ex;
            }
            catch (...) {
                _ret = false;
                _ret.error = "File writer returns unknown error";
            }

            if (!_ret) {
                _stopFlag = true;
            }

        };

        return fileWriterJob;
    }

    std::function<void()> FileToFileTask::prepareWorkerJob(std::shared_ptr<twPro::IWorker> _worker, std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret)
    {
        auto workerJob = [_worker, &_stopFlag, &_ret]() {

            try {
                _worker->work(_stopFlag);
            }
            catch (const std::exception& ex) {
                _ret = false;
                _ret.error = ex.what();
            }
            catch (const std::string& ex) {
                _ret = false;
                _ret.error = ex;
            }
            catch (...) {
                _ret = false;
                _ret.error = "Worker returns unknown error";
            }

            if (!_ret) {
                _stopFlag = true;
            }

        };

        return workerJob;
    }

    void FileToFileTask::setupNotifiers(twPro::DataChannel & _channel, std::atomic_bool & _stopFlag, SourceToResultTemplateTask::_pResult & _ret)
    {
        auto writerNotifier = _channel.createNotifier<size_t>(OPTIMAL_NOTIFIER_QUEUE_SIZE);
        auto progress = interactive()->createProgressBar();

        m_resultFilePtr->setNotifier_currentConsumedDataUnits(writerNotifier);

        writerNotifier->setCallBack([this, &_stopFlag, progress](const size_t & _val) {

            progress->notify(IInteractive::Progress("Progress", _val, totalWriteDataUnits()));

            if (_val >= totalWriteDataUnits()) {
                _stopFlag = true;
            }

        });
    }

    void FileToFileTask::clear()
    {
        m_sourceFilePtr.reset();
        m_resultFilePtr.reset();
    }

    size_t FileToFileTask::totalData() const
    {
        return  m_sourceFilePtr->totalData();
    }

    size_t FileToFileTask::totalWriteDataUnits() const
    {
        return (totalData() / parameters().blockSize) + (totalData() % parameters().blockSize > 0 ? 1 : 0);
    }

}
